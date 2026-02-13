#pragma once

#include "storage/schema.h"

#include <vector>
#include <string>

class Tuple {
    public: 
    /** Player Exclusive 
    struct Player {
        std::string name;
        uint16_t yob; 
        uint16_t major; 
        Player(){}
        Player(const std::string name, uint16_t yob, uint16_t major) : name(name), yob(yob), major(major) {}
    }; 
    static std::vector<char> serialize(const Player& player) {
        uint16_t name_length = static_cast<uint16_t>(player.name.length()); 
        size_t size = sizeof(uint16_t) + name_length + sizeof(uint16_t) + sizeof(uint16_t); 

        std::vector<char> buffer(size); 
        size_t offset = 0; 

        std::memcpy(buffer.data() + offset, &name_length, sizeof(uint16_t)); 
        offset += sizeof(uint16_t); 

        std::memcpy(buffer.data() + offset, player.name.c_str(), name_length); //c_str to convert string to char*
        offset += name_length; 

        std::memcpy(buffer.data() + offset, &player.yob, sizeof(uint16_t)); 
        offset += sizeof(player.yob); 

        std::memcpy(buffer.data() + offset, &player.major, sizeof(uint16_t)); 
        offset += sizeof(player.major); 

        return buffer; 

    }

    static Player deserailize(const std::string raw_bytes) {
        const char* data = raw_bytes.data(); 
        Player player; 
        size_t offset = 0; 

        uint16_t name_length; 
        std::memcpy(&name_length, data + offset, sizeof(uint16_t)); 
        offset += sizeof(uint16_t); 

        player.name.assign(data + offset, name_length); //assign to 
        offset += name_length; 

        std::memcpy(&player.yob, data + offset, sizeof(uint16_t)); 
        offset += sizeof(uint16_t); 

        std::memcpy(&player.major, data + offset, sizeof(uint16_t)); 
        offset += sizeof(uint16_t); 

        return player; 

    }
        **/
    static std::vector<char> serialize(const Schema& schema, const std::vector<std::string>& values);
    static std::vector<std::string> deserialize(const Schema& schema, const std::string& raw_bytes);
}; 