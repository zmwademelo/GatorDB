#include "storage/disk_manager.h"
#include <iostream>
#include <cerrno>

DiskManager::DiskManager(const std::string& db_file) : file_name_(db_file){
    // Try to open in read/write mode
    db_io_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);

    // If file doesn't exist, create it
    if(!db_io_.is_open()){
        db_io_.open(db_file, std::ios::binary | std::ios::out);
        
        // Write an empty page to ensure file has content
        char empty_page[PAGE_SIZE] = {0};
        db_io_.write(empty_page, PAGE_SIZE);
        db_io_.close();
        
        // Now reopen in read/write mode
        db_io_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);
        
        if(!db_io_.is_open()){
            std::cerr << "Error: Failed to open/create file " << db_file << std::endl;
        }
    }

}

DiskManager::~DiskManager(){
    if (db_io_.is_open()) db_io_.close();
}

bool DiskManager::write_page(page_id_t page_id, const char* data){
    if(!db_io_.is_open()){
        std::cerr << "Error: File is not open for writing!" << std::endl;
        return false;
    }
    
    // Clear any existing error flags
    db_io_.clear();
    
    size_t offset = page_id * PAGE_SIZE;
    db_io_.seekp(offset);
    
    if(db_io_.fail()){
        std::cerr << "Error: seekp failed! Offset: " << offset << ", State: " << db_io_.rdstate() << std::endl;
        return false;
    }
    
    db_io_.write(data, PAGE_SIZE);
    db_io_.flush();
    
    if(db_io_.fail()){
        std::cerr << "Error: Write/flush failed! State flags: " << db_io_.rdstate() << std::endl;
        return false;
    }
    return true; 
}

//page_id_t page_id: This is a uint32_t representing which 4KB block you want.
//char* data: This is a pointer to the destination memory in RAM where the disk data will be copied. It is not const because the function needs to modify (fill) this memory.
bool DiskManager::read_page(page_id_t page_id, char* data){
    size_t offset = page_id * PAGE_SIZE; //A variable that stores the exact "byte address" in the file.
    //This moves the file's internal pointer to the correct position.
    db_io_.seekg(offset); //If you want Page #2, the offset is 2 times 4096 = 8192$. This tells the computer to skip the first 8192 bytes of the file to reach the start of the second page.
    //This reads 4096 bytes from the file into the memory pointed to by 'data'. data is the pointer for destination. PAGE_SIZE is the chunk size to read.
    db_io_.read(data, PAGE_SIZE); //The first parameter tells the function where to put those bytes, and the second parameter tells it how many bytes to read (4096 bytes for a standard page).
    if(db_io_.fail()){
        return false; 
    }
    return true; 
}

//? This function calculates the total size of the database file by seeking to the end and using tellg() to get the current position, which corresponds to the file size in bytes. This is useful for determining how many pages are currently in the file and for managing page allocations.
uint64_t DiskManager::get_file_size() {
    db_io_.seekg(0, std::ios::end); // Move to the end of the file
    return db_io_.tellg(); // Return the current position, which is the file size
}