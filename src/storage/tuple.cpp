#include "tuple.h"
#include "schema.h"

#include <string> 
#include <vector>

std::vector<char> Tuple::serialize(const Schema& schema, const std::vector<std::string>& values){

    uint32_t designed_size = schema.get_designed_size(); 
    std::vector<char> buffer(designed_size); //The buffer holds the largest possible record size based on the schema. 

    const std::vector<Column>& columns = schema.get_columns(); //columns is a vector of <Column>. 
    for (size_t i = 0; i < columns.size(); ++i) {
        if (columns[i].type == Type::INTERGER) {
            int val = std::stoi(values[i]); //stoi converts string to int
            //The vector::data() function in C++ returns a direct pointer to the underlying contiguous array of elements managed by the std::vector. 
            //This allows the vector's elements to be accessed or modified using raw pointer arithmetic or passed to functions that expect a pointer to an array. 
            std::memset(buffer.data() + columns[i].offset, 0, sizeof(int)); //Clear the 4 bytes we are about to write to. 
            std::memcpy(buffer.data() + columns[i].offset, &val, sizeof(int)); //Use &val to get the memory address of val, then copy 4 bytes to the buffer. 
        } else if (columns[i].type == Type::VARCHAR) {

            //Clear the bytes we are about to write to, in case the new string is shorter than the previous one. 
            std::memset(buffer.data() + columns[i].offset, 0, columns[i].length); 
            //This serves as a safeguard to prevent buffer overflow if the input string is longer than the defined length for that column in the schema so that max bytes it takes is the length defined in the schema.
            uint32_t varchar_bytes_to_copy = std::min((uint32_t)values[i].length(), columns[i].length); 
            std::memcpy(buffer.data() + columns[i].offset, values[i].c_str(), varchar_bytes_to_copy); 
        }
    }
    return buffer; 
}

std::vector<std::string> Tuple::deserialize(const Schema& schema, const std::string& raw_bytes){ //Why not const reference for raw_bytes? 

    std::vector<std::string> buffer; //The buffer is default initilized meaning it's empty and we will push_back deserialized values into it.

    const std::vector<Column>& columns = schema.get_columns();
    const char* data = raw_bytes.data(); //Get a pointer to the raw byte date of the record. 

    for (const Column& col : columns) {
        if (col.type == Type::INTERGER) {
            int val; 
            // Jump to the fixed offset and copy 4 bytes into an int
            std::memcpy(&val, data + col.offset, sizeof(int)); 
            buffer.push_back(std::to_string(val)); 
        } else if (col.type == Type::VARCHAR) {
            std::string str(data + col.offset, col.length); //Initialize a string using (pointerm length) constructor to read the fixed length of bytes for the varchar column.
            // Clean up: Remove trailing null bytes so "Gator\0\0\0" becomes "Gator"
            size_t first_null = str.find('\0'); 
            if (first_null != std::string::npos) {
                str.resize(first_null); //If a null byte is found, resize the string to end at the first null byte, effectively trimming any padding nulls from the end of the string.
            }

            buffer.push_back(str); 
        }
    }
    
    return buffer; 
}

