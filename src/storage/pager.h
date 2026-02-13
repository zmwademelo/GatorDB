#pragma once

#include "common/config.h"
#include "disk_manager.h"
#include "table_page.h"
#include "schema.h"


/**
New File: If the file is empty, initialize a FileHeader with default values (like next_page_id = 1 if Page 0 is the header) and use DiskManager::WritePage to save it.

Existing File: Use DiskManager::ReadPage(0, ...) to load the header into memory. Immediately verify the magic_number to ensure the file is a valid GatorDB format.
*/
struct FileHeader {
        //MVP version FileHeader
        uint32_t magic_number_ =  GATORDB_MAGIC_NUMBER; // "GATR" in ASCII
        uint32_t next_page_id = 1; //
        uint32_t record_count = 0; 
        
       /*
       uint32_t magic_number_ =  GATORDB_MAGIC_NUMBER; // "GATR" in ASCII
       uint32_t version_ = 1;
       uint32_t table_count = 0; 
        */
}; 

/**
Function of Pager class: 
1. Manage the file header, which includes metadata like the next available page ID and the total record count. This header is stored in Page 0 of the file. 
2. Provide a Pager object to the Executer, which will use it to read/write pages and manage record counts. 

*/
class Pager {
    public: 
        // Frog: Ownership of DiskManager is not clear. Consider using unique_ptr if Pager is responsible for managing the lifetime of DiskManager (i.e. "owns" it).
        explicit Pager(DiskManager* diskmanager); //Explicit constructor to prevent implicit conversions. It takes a pointer to a DiskManager, which it will use for all disk operations.

        Schema* get_schema_directory(){return reinterpret_cast<Schema*>(&header_ + sizeof(FileHeader));}

        // Frog: Add a comment. This method's semantics is not very intuitive. For example, if page 1 has 100 remaining bytes, and page 2 has 200 remaining bytes, what will be returned when required_space is 150? How about 50?
        page_id_t get_available_page_id(uint16_t required_space); 
        
        // page_id_t get_page_count();

        // Frog: This is error-prone. What if the `data` in `write_page` is not exactly PAGE_SIZE bytes (undefined behavior)? Similarly, what if `data` in `read_page` is not large enough to hold PAGE_SIZE bytes?
        bool write_page(page_id_t page_id, const char* data);
        bool read_page(page_id_t page_id, char* data);

        //MVP versiobn only 
        uint32_t get_total_records() const { return header_.record_count; };
        // Frog: Error-prone as well. Writing to page and updating page count should be atomic operations. Consider combining these into a single method.
        void increment_global_record_count(); 
        void decrement_global_record_count();

        FileHeader* get_file_header() {
            return &header_; 
        }

    private:
        FileHeader header_;
        DiskManager* disk_manager_;

        page_id_t allocate_new_page(); 
        void load_header();   //Loads and validates the header on startup
        void flush_header();  //Persists the header to page 0
}; 