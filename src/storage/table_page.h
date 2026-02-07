#pragma once
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
