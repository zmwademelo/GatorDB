#pragma once

#include "common/config.h"
#include "disk_manager.h"
#include "schema.h"

#include <unordered_map>
#include <iostream>
#include <memory.h>
#include <cstring>

/**
New File: If the file is empty, initialize a FileHeader with default values (like next_page_id = 1 if Page 0 is the header) and use DiskManager::WritePage to save it.

Existing File: Use DiskManager::ReadPage(0, ...) to load the header into memory. Immediately verify the magic_number to ensure the file is a valid GatorDB format.

// Frog: It looks like the above comment should be moved to some Pager method, not for FileHeader.
*/
struct FileHeader
{
    uint32_t magic_number = GATORDB_MAGIC_NUMBER; // "GATR" in ASCII
    uint32_t version = 1;
    uint32_t catalog_page_head = 1;
    uint32_t catalog_page_tail = 1; // Points to LAST catalog page (for appending)
    uint32_t page_count = 1;        // page 0 and page 1 are initialized. This includes free pages
    uint32_t free_page_head = INVALID_PAGE_ID;
    uint32_t free_page_tail = INVALID_PAGE_ID;

    // The padding takes remainder of Page 0
    char padding[PAGE_SIZE - 7 * sizeof(uint32_t)];
};

struct PageHeader
{
    uint32_t magic_number = GATORDB_MAGIC_NUMBER;
    page_id_t prev_page_id = INVALID_PAGE_ID;
    page_id_t next_page_id = INVALID_PAGE_ID;
    uint16_t lower_bound = sizeof(PageHeader);
    uint16_t upper_bound = PAGE_SIZE;
    uint16_t slot_count = 0; //This includes deleted slots
    uint16_t deleted_count = 0;
    //Currently the free space does not count space freed by "deleting" a slot. 
    uint16_t GetFreeSpace() { return upper_bound - lower_bound; }
};

struct Slot
{
    uint16_t offset;
    int16_t length; // negative length means the slot is deleted
};

struct TableMetadata
{
    std::string table_name;
    Schema schema;
    page_id_t first_page_id;
    // page_id_t last_page_id;

    TableMetadata(std::string table_name, Schema schema, page_id_t first_page_id) : table_name(table_name), schema(schema), first_page_id(first_page_id) {}
};
// Not used for now
struct Page
{
    page_id_t id;         // Which page is this? (0, 1, 2...)
    bool is_dirty;        // Has this data been modified in memory?
    char data[PAGE_SIZE]; // The actual 4096 bytes of "real estate"

    // Helper to give pointers to other classes
    char *GetData() { return data; }
    void clear() { std::memset(data, '0', PAGE_SIZE); }
};

class Value
{
private:
    Type type_;
    // A union lets us store an int OR a pointer to a string in the same space
    union
    {
        int32_t integer_;
        char *string_;
    } data_;

public:
    auto GetData() const { return data_; } // const
    auto GetType() const { return type_; }
    Value(int32_t val) : type_(Type::INTEGER)
    {
        data_.integer_ = val;
    }
    Value(const std::string &val) : type_(Type::VARCHAR)
    {
        data_.string_ = new char[val.length() + 1]; // Allocate memory for the string value, +1 for the null terminator
        std::strcpy(data_.string_, val.c_str());    // Copy the string data into our union
    }
    // static Value Deserialize(const char* buffer, Type type) = delete;

    bool operator==(const Value& other) const {
        if (type_ != other.type_) return false;
        if (type_ == Type::INTEGER) return data_.integer_ == other.data_.integer_;
        if (type_ == Type::VARCHAR) return std::strcmp(data_.string_, other.data_.string_) == 0;
        return false;
    }
};

// Frog: Add a class comment!
class Pager
{
public:
    static std::unique_ptr<Pager> Create(const std::string db_file_name);

    const FileHeader& GetFileHeader() const { return file_header_; }


    //Page management
    page_id_t AllocateNewPage();
    void DeallocatePage(page_id_t page_id);
    // void mark_dirty(page_id_t page_id);
    // void flush_page(page_id_t page_id, const char* data);
    // void flush_all_pages();

    //Disk operations
    std::vector<char> ReadPage(page_id_t page_id) const;
    bool WritePage(page_id_t page_id, const char *data) const;

    //Record CRUD based on pages
    uint16_t InsertRecord(const std::vector<char> &record, page_id_t page_id) const;
    std::vector<char> GetRecordRawBytes(page_id_t page_id, uint16_t slot_num) const;
    bool DeleteRecord(page_id_t page_id, uint16_t slot_num) const; // If slot_count = 0 deallocate page

    //Used to be TablePage functions
    // PageHeader get_page_header(page_id_t page_id);
    // Slot* get_slot_directory(page_id_t page_id);

private:
    explicit Pager(std::unique_ptr<DiskManager> diskmanager); // Explicit constructor to prevent implicit conversions. It takes a pointer to a DiskManager, which it will use for all disk operations.

    FileHeader file_header_;
    std::unique_ptr<DiskManager> disk_manager_;
    // The Pager should maintain a collection of Page objects in memory. When you call fetch_page, the Pager checks if it already has that page in RAM. If it does, it returns it instantly; if not, it goes to the disk.
    std::unordered_map<page_id_t, std::unique_ptr<Page>> buffer_pool_; // Caches pages in RAM.

    // Loads and validates the header on startup
    bool LoadFileHeader(); 
    void InitFileHeader();
    void InitPage(page_id_t page_id); 
    // Persists the header to page 0
    void FlushFileHeader(); 
};