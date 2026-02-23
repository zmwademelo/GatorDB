#pragma once

#include <string>
#include <vector> 

enum class Type {INTEGER, VARCHAR, UNKNOWN};

struct Column {
    std::string name; 
    Type type; 
    uint32_t length; 
    uint32_t offset; // This will be calculated when we initialize the Schema based on the order of columns and their types

    Column() {
        name = "";
        type = Type::UNKNOWN; 
        length = 0; 
        offset = 0; 
    }
};


class Schema {
    public: 
    Schema(){}
    //Use explicit for any constructor that takes a single argument unless you have a specific, documented reason to allow implicit conversion (like a custom String class converting from const char*). 
    explicit Schema(std::vector<Column> columns) : columns_(std::move(columns)) { //use std::mmove to avoid unnecessary copying of the columns vector, since we won't need the original after constructing the Schema.
        uint32_t current_offset = 0; 
        for (auto& col : columns_) {    //Use a reference to avoid copying each Column struct, and non-const because we need to modify the offset field of each column.
            col.offset = current_offset; 
            //Supports int and varchar for now
            current_offset += (col.type == Type::INTEGER) ? sizeof(int) : col.length;    //4 is the hardocded byte size for integer. 
        }
        designed_size_ = current_offset; //Designed size is calculated during initialization and stored as a member variable for quick access later.
    }
    //Quick access functions
    const std::vector<Column>& GetColumns() const {return columns_; } //Return a const reference to avoid unnecessary copying of the columns vector when we just want to read it.
    uint32_t GetDesignedSize() const {return designed_size_; } 
    uint32_t GetColumnOffset(size_t col_num) const {return columns_[col_num].offset; }
    Type  GetColumnType(size_t col_num) const {return columns_[col_num].type; }

    private: 
    std::vector<Column> columns_; 
    uint32_t designed_size_; 
}; 

