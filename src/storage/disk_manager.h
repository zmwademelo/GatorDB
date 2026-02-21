#pragma once
#include <string>
#include <fstream>
#include "common/config.h"

// Frog: Add a class comment!
class DiskManager {
    public: 
        DiskManager(const std::string& db_file);
        ~DiskManager();

        // Writes a 4KB buffer to a specific page_id
        //page_id is the page number you want to write to. data is a pointer to the memory in RAM where the data to be written is stored.
        // Frog: Highlight that data must contain the right size of valid memory.
        // Frog: You can use `` to differentiate code and word. i.e. `data` is a pointer ...
        bool write_page(page_id_t page_id, const char* data);//Use char because we are dealing with raw bytes. And char epresents the most basic unit of addressable memory: a single byte. 

        // Reads a 4KB buffer from a specific page_id
        // Frog: Ditto: valid memory of data
        bool read_page(page_id_t page_id, char* data);
        uint64_t get_file_size();

    private: 
        std::fstream db_io_;
        // Frog: If you store the file_name string, you'd better take a string rather that const string& in constructor. This way, you can just move the string
        // in, instead of creating a copy.
        std::string file_name_; 
};