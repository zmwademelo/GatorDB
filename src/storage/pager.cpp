#include "pager.h"

Pager::Pager(DiskManager* diskmanager) : disk_manager_(diskmanager) {
    // Frog: Minimize logic in constructor
        load_header(); // Only load if the file already exists and has data
}

void Pager::load_header() {
    char buffer[PAGE_SIZE] = {0}; 
    if (disk_manager_->read_page(0, buffer) && buffer[0] != '\0') { // Check if read was successful and buffer is not empty
        std::memcpy(&header_, buffer, sizeof(header_)); 
        if (header_.magic_number_!= GATORDB_MAGIC_NUMBER) {
            throw std::runtime_error("Invalid database file: Magic number mismatch. ");
        }
    }
    else {
        header_ = FileHeader(); 
        flush_header(); 
        if (disk_manager_->read_page(0, buffer) && buffer[0] != '\0') { // Check if read was successful and buffer is not empty
        std::memcpy(&header_, buffer, sizeof(header_)); 
        if (header_.magic_number_!= GATORDB_MAGIC_NUMBER) {
            throw std::runtime_error("Invalid database file: Magic number mismatch. ");
        }
        allocate_new_page(); // Allocate page 1 for the first TablePage
    }
    }

}

void Pager::flush_header() {
    char buffer[PAGE_SIZE] = {0}; // Initialize buffer to zero
    std::memcpy(buffer, &header_, sizeof(header_));
    disk_manager_->write_page(0, buffer);
}



bool Pager::write_page(page_id_t page_id, const char* data) {
    bool success = disk_manager_->write_page(page_id, data); 
    if (!success) {
        throw std::runtime_error("Failed to write page to disk.");
    }
    return success; 
}

bool Pager::read_page(page_id_t page_id, char* data) {
    bool success = disk_manager_->read_page(page_id, data);
    if (!success) {
        throw std::runtime_error("Failed to read page from disk.");
    }
    return success; 
}

void Pager::increment_global_record_count() {
    header_.record_count++;
    flush_header(); // Update the header on disk whenever the record count changes
}

void Pager::decrement_global_record_count() {
    if (header_.record_count > 0) {
        header_.record_count--;
        flush_header(); // Update the header on disk whenever the record count changes
    }
}

page_id_t Pager::allocate_new_page() {
    // 1. Capture the current available ID
    page_id_t assigned_id = header_.next_page_id;

    // 2. Increment the ID for the NEXT call
    header_.next_page_id++;

    // 3. Persist the change to Page 0 immediately
    flush_header(); 

    char new_page_buffer[PAGE_SIZE] = {0}; // Create a new page with zero-initialized data
    TablePage new_page = TablePage(new_page_buffer); // Create a new page with zero-initialized data
    new_page.initialize_empty_page(); // Set up the page header and slot directory
    std::memcpy(new_page_buffer, new_page.data_, PAGE_SIZE); // Copy the initialized page data into the buffer
    write_page(assigned_id, new_page_buffer); // Write the new page to disk

    return assigned_id;
}

page_id_t Pager::get_available_page_id(uint16_t required_space){
    for (auto page_id = 1; page_id < header_.next_page_id; ++ page_id) {
        char buffer[PAGE_SIZE];
        disk_manager_->read_page(page_id, buffer);
        TablePage page(buffer); 

        auto header = page.get_page_header();
        if (header == nullptr) {
            page.initialize_empty_page();
            disk_manager_->write_page(page_id, page.data_); //? 
        }
        if (header->free_space >= required_space) {
            return page_id;
        }
    }
    return allocate_new_page();
}