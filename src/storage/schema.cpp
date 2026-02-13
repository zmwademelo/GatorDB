#include "schema.h"

#include <string>
#include <vector> 

/*
//total_size represents the exact number of bytes required to store one full record (tuple) in memory or on disk.
uint32_t Schema::get_total_size() const {
    uint32_t current_offset = 0; 
        for (auto& col : columns_) {
            col.offset = current_offset; 
            //Supports int and varchar for now
            current_offset += col.length;
        }
    return current_offset; 
}

*/