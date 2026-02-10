#pragma once
#include <string>
#include <fstream>
#include "common/config.h"

class DiskManager {
    public: 
        DiskManager(const std::string& db_file); //db_file is the name of the database file on disk
        ~DiskManager();

        // Writes a 4KB buffer to a specific page_id
        //page_id is the page number you want to write to. data is a pointer to the memory in RAM where the data to be written is stored.
        bool write_page(page_id_t page_id, const char* data);//Use char because we are dealing with raw bytes. And char epresents the most basic unit of addressable memory: a single byte. 

        // Reads a 4KB buffer from a specific page_id
        bool read_page(page_id_t page_id, char* data);
        uint64_t get_file_size();

    private: 
        std::fstream db_io_;
        std::string file_name_; 
};