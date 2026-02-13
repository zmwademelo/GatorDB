#include "table_page.h"
#include <string>
/*
Function of TablePage class: 
1. Take Pager, TablePage and StatementParser as input, and perform the actual record-level operations like insert, select, and delete.
*/

void TablePage::initialize_empty_page() {
    auto header = get_page_header();
    header->magic_number = GATORDB_MAGIC_NUMBER;
    header->upper_bound = PAGE_SIZE; 
    header->lower_bound = sizeof(PageHeader);
    header->slot_count = 0; 
    header->free_space = PAGE_SIZE - sizeof(PageHeader);
}

uint16_t TablePage::insert_record(const char* record_data, uint16_t record_length) { //Accept length to prevent only the first char is passed
    auto header = get_page_header();
    auto slots = get_slot_directory();
    //uint16_t record_length = std::strlen(record_data); //Apply strlen to c style string to get the length of the record data.
    uint16_t required_space = record_length + sizeof(Slot); //Calculate the total space needed
    if (header->free_space < required_space) {
        return -1;
    }
    //reuse deleted slot if available
    for (uint16_t i = 0; i < header->slot_count; ++i) {
        if (slots[i].length < 0 && std::abs(slots[i].length) >= record_length) {
            uint16_t offset = slots[i].offset;
            std::memcpy(data_ + offset, record_data, record_length); 
            slots[i].length = record_length; // Update the slot to reflect the new record length
            header->free_space -= record_length; // Decrease free space by the size of the new record
            return i; // Return the slot number where the record was inserted
        }
    }
    // Insert new record at the upper bound
    uint16_t offset = header->upper_bound - record_length; 
    std::memcpy(data_ + offset, record_data, record_length);
    // Update the slot directory
    uint16_t new_slot_num = header->slot_count;
    slots[new_slot_num].offset = offset; 
    slots[new_slot_num].length = record_length;
    // Update the page header
    header->upper_bound = offset;
    header->free_space -= required_space;
    header->lower_bound += sizeof(Slot);
    header->slot_count++; 
    return new_slot_num; // Return the slot number where the record was inserted
}

bool TablePage::delete_record(uint16_t slot_num) {
    auto header = get_page_header(); 
    /*
    auto result = get_slot(slot_num); 
    if (!result.ok()) {
        std::cout << "Error: " << result.status().message() << std::endl;
        return false; 
    }
    */
    Slot* slot = get_slot(slot_num); 
    if (slot == nullptr || slot->length < 0) { // Invalid slot number or already deleted
        return false;
    }
    
    //auto slot = *std::move(result);
    slot->length = -slot->length; // Mark the slot as deleted by negating the length
    
    //TBD This does not work cuz get_available_page uses free space to determine if a new page is needed. The block needs to be continuous. 
    //header->free_space += (-slot->length); // Increase free space by the size of the deleted record
    
    return true;
    
}

std::string TablePage::get_record(uint16_t slot_num) {
    auto header = get_page_header(); 
    /*
    auto result = get_slot(slot_num); 
    if (!result.ok()) {
        std::cout << "Error: " << result.status().message() << std::endl;
        return ""; 
    }
        */
    
    Slot* slot = get_slot(slot_num); 
    if (slot == nullptr || slot->length < 0) { // Invalid slot number or deleted record
        std::cout << "Error: Record not found or has been deleted. " << std::endl;
        return ""; 
    }
    
    //auto slot = *std::move(result);
    return std::string(data_ + slot->offset, slot->length); // Return a substring based on the offset and length. 
}

