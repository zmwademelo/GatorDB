#include "pager.h"

Pager::Pager(DiskManager* diskmanager) : disk_manager_(diskmanager) {
    load_header();
}

void Pager::load_header() {
    char buffer[PAGE_SIZE]; //memset? --- IGNORE ---
    disk_manager_->read_page(0, buffer); 
    std::memcpy(&header_, buffer, sizeof(header_)); //sizeof is a compile-time operator that returns the size of a type in bytes. 
    //It is used here to ensure that we copy the correct number of bytes from the buffer into the header_ structure. 
    if (sizeof(header_) == 0) {
        header_ = FileHeader(); // Initialize with default values if the header is empty
        flush_header(); // Write the initialized header to disk
    }
    if (header_.magic_number_ != GATORDB_MAGIC_NUMBER) {
        throw std::runtime_error("Invalid file format: Magic number does not match.");
    }
}

void Pager::flush_header() {
    char buffer[PAGE_SIZE]; 
    std::memcpy(buffer, &header_, sizeof(header_));
    disk_manager_->write_page(0, buffer);
}

page_id_t Pager::allocate_page() {
    page_id_t next_page_id = header_.next_page_id;
    increment_record_count(); // Increment the record count for the new page
    return next_page_id;
}

void Pager::write_page(page_id_t page_id, const char* data) {
    disk_manager_->write_page(page_id, data);
}

void Pager::read_page(page_id_t page_id, char* data) {
    disk_manager_->read_page(page_id, data);
}

void Pager::increment_record_count() {
    header_.record_count++;
    flush_header(); // Update the header on disk whenever the record count changes
}

