#pragma once

#include "common/config.h"
#include "disk_manager.h"
#include "schema.h"

#include <unordered_map>
#include <iostream>

/**
New File: If the file is empty, initialize a FileHeader with default values (like next_page_id = 1 if Page 0 is the header) and use DiskManager::WritePage to save it.

Existing File: Use DiskManager::ReadPage(0, ...) to load the header into memory. Immediately verify the magic_number to ensure the file is a valid GatorDB format.
*/
struct FileHeader {
       
        uint32_t magic_number_ =  GATORDB_MAGIC_NUMBER; // "GATR" in ASCII
        uint32_t version_ = 1;
        uint32_t catalog_page_head_ = 1; 
        uint32_t catalog_page_tail_ = 1;    // Points to LAST catalog page (for appending)
        uint32_t page_count_ = 1; //page 0 and page 1 are initialized. This includes free pages
        uint32_t free_page_head_ = INVALID_PAGE_ID; 
        uint32_t free_page_tail_ = INVALID_PAGE_ID; 

        char padding[PAGE_SIZE - 7 * sizeof(uint32_t)]; 

}; 

// struct CatalogPageHeader {
//     uint32_t magic_number;
//     uint16_t num_entries;           // How many tables stored here
//     uint16_t free_space;            // Remaining space
//     page_id_t next_catalog_page_id; // 0 if this is the last page
// };


struct PageHeader {
            uint32_t magic_number = GATORDB_MAGIC_NUMBER; 
            page_id_t prev_page_id = INVALID_PAGE_ID; 
            page_id_t next_page_id = INVALID_PAGE_ID; 
            uint16_t lower_bound = sizeof(PageHeader);
            uint16_t upper_bound = PAGE_SIZE; 
            uint16_t slot_count = 0; //includes deleted slot
            uint16_t deleted_count = 0; 

            uint16_t get_free_space() {return upper_bound - lower_bound; }
        };

struct Slot{
    uint16_t offset; 
    int16_t length; //negative length means the slot is deleted
}; 


struct TableMetadata {
    std::string table_name; 
    Schema schema; 
    page_id_t first_page_id; 
    //page_id_t last_page_id; 

    TableMetadata(std::string table_name, Schema schema, page_id_t first_page_id) : table_name(table_name), schema(schema), first_page_id(first_page_id) {}
};

struct Page {
    page_id_t id;             // Which page is this? (0, 1, 2...)
    bool is_dirty;            // Has this data been modified in memory?
    char data[PAGE_SIZE];     // The actual 4096 bytes of "real estate"
    
    // Helper to give pointers to other classes
    char* get_data() { return data; }
    void clear() { std::memset(data, '0', PAGE_SIZE); }
};

class Value {
private:
    Type type_;
    // A union lets us store an int OR a pointer to a string in the same space
    union {
        int32_t integer_;
        char* string_; 
    } data_;
public: 
    auto get_data() const {return data_; } //const
    auto get_type() const {return type_; }
    Value (int32_t val) : type_(Type::INTEGER) {
        data_.integer_ = val; 
    }
    Value (const std::string &val) : type_(Type::VARCHAR) {
        data_.string_ = new char[val.length() + 1]; //Allocate memory for the string value, +1 for the null terminator
        std::strcpy(data_.string_, val.c_str()); //Copy the string data into our union
    }
    static Value deserialize(const char* buffer, Type type); 
};


class Pager {
    public: 


        // Frog: Ownership of DiskManager is not clear. Consider using unique_ptr if Pager is responsible for managing the lifetime of DiskManager (i.e. "owns" it).
        explicit Pager(DiskManager* diskmanager); //Explicit constructor to prevent implicit conversions. It takes a pointer to a DiskManager, which it will use for all disk operations.

        FileHeader* get_file_header() {
            return &header_; 
        }

        page_id_t allocate_new_page(); 

        void deallocate_page(page_id_t page_id);
        // void mark_dirty(page_id_t page_id); 
        // void flush_page(page_id_t page_id, const char* data);
        // void flush_all_pages(); 

        std::vector<char> read_page(page_id_t page_id); 
        bool write_page(page_id_t page_id, const char* data); 
     
        uint16_t insert_record(std::vector<char> record, page_id_t page_id); 
        std::vector<char> get_record_raw_bytes(page_id_t page_id, uint16_t slot_num); 
        bool delete_record(page_id_t page_id, uint16_t slot_num); //If slot_count = 0 deallocate page

        //PageHeader get_page_header(page_id_t page_id); 
        //Slot* get_slot_directory(page_id_t page_id); 
        


    private:
        FileHeader header_;
        //PageHeader page_header_;
        DiskManager* disk_manager_; 
        //The Pager should maintain a collection of Page objects in memory. When you call fetch_page, the Pager checks if it already has that page in RAM. If it does, it returns it instantly; if not, it goes to the disk.
        std::unordered_map<page_id_t, std::unique_ptr<Page>> buffer_pool_; //Caches pages in RAM.

        
        bool load_file_header();   //Loads and validates the header on startup
        void init_file_header(); 
        void init_page(page_id_t page_id); 

        void flush_header();  //Persists the header to page 0

        //PageHeader* get_page_header(page_id_t page_id);

 

}; 