#pragma once
#include "disk_manager.h"
#include "table_page.h"
#include "common/config.h"
/*
New File: If the file is empty, initialize a FileHeader with default values (like next_page_id = 1 if Page 0 is the header) and use DiskManager::WritePage to save it.

Existing File: Use DiskManager::ReadPage(0, ...) to load the header into memory. Immediately verify the magic_number to ensure the file is a valid GatorDB format.
*/
struct FileHeader {

        uint32_t magic_number_ =  GATORDB_MAGIC_NUMBER; // "GATR" in ASCII
        uint32_t next_page_id = 1; 
        uint32_t record_count = 0; 
}; 

class Pager {
    public: 
        explicit Pager(DiskManager* diskmanager); 

        page_id_t allocate_page(); 

        // page_id_t get_page_count();

        void write_page(page_id_t page_id, const char* data){};
        void read_page(page_id_t page_id, char* data){};

        uint32_t get_next_page_id() const { return header_.next_page_id; }
        void increment_record_count(); 

    private:
        FileHeader header_;
        DiskManager* disk_manager_;
        void load_header();   //Loads and validates the header on startup
        void flush_header();  //Persists the header to page 0
}; 