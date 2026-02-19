#include "pager.h"
#include "table_page.h"

Pager::Pager(DiskManager* diskmanager) : disk_manager_(diskmanager) {

        if (!load_file_header()) {
            init_file_header(); 
            allocate_new_page(); 
        }
              
}

bool Pager::load_file_header() {
    char buffer[PAGE_SIZE] = {0}; 
    bool header_exists = disk_manager_->read_page(0, buffer) && buffer[0] != '\0';
    
    if (header_exists) {
        // Existing database file - load and validate header
        std::memcpy(&header_, buffer, sizeof(header_)); 
        if (header_.magic_number_ != GATORDB_MAGIC_NUMBER) {
            throw std::runtime_error("Invalid database file: Magic number mismatch.");
        } else {
            return true; 
        }
    } else {
        return false; 
    }
}

void Pager::init_file_header(){
    header_ = FileHeader(); 
    flush_header(); 
}
    

void Pager::flush_header() {
    char buffer[PAGE_SIZE] = {0}; // Initialize buffer to zero 
    std::memcpy(buffer, &header_, sizeof(header_));
    disk_manager_->write_page(0, buffer);
}

void Pager::init_page(page_id_t page_id) {
    if (page_id < 1) return; 
    char buffer[PAGE_SIZE] = {0};
    PageHeader page_header = PageHeader(); 
    std::memcpy(buffer, &page_header, sizeof(PageHeader)); 
    disk_manager_->write_page(page_id, buffer); 
}

//2.0 

page_id_t Pager::allocate_new_page() {
    page_id_t new_page_id; 
    char page_buffer[PAGE_SIZE] = {0};
    if (header_.free_page_head_ > 0 && header_.free_page_head_ != INVALID_PAGE_ID) { //Recycled
        new_page_id = header_.free_page_head_; 

        std::vector<char> free_page_head_buffer = read_page(new_page_id); 
        PageHeader* free_page_head_header = reinterpret_cast<PageHeader*>(free_page_head_buffer.data()); 

        if (header_.free_page_head_ == header_.free_page_tail_) { //only one free page
            header_.free_page_head_ = INVALID_PAGE_ID; 
        }else {
            header_.free_page_head_ = free_page_head_header->next_page_id;
        }
    }else{
        new_page_id = header_.page_count_++; 
        //When you grow the file, you should ideally write a "blank" page (4096 zeros) to that new offset immediately to "claim" the space.
        init_page(new_page_id); 
    }
    
    flush_header(); 
    return new_page_id; 
}

void Pager::deallocate_page(page_id_t page_id) {

    if (page_id < 2) return; //Page 0 and Page 1 are reserved
    std::vector<char> page_buffer = read_page(page_id); 
    PageHeader* page_header = reinterpret_cast<PageHeader*>(page_buffer.data()); 
    if (header_.free_page_head_ == INVALID_PAGE_ID) {
        //Assign free page head first time
        page_header->magic_number = GATORDB_MAGIC_NUMBER; 
        page_header->prev_page_id = INVALID_PAGE_ID; 
        page_header->next_page_id = INVALID_PAGE_ID; 
        page_header->lower_bound = sizeof(PageHeader);
        page_header->upper_bound = PAGE_SIZE; 
        page_header->slot_count = 0;
        page_header->deleted_count = 0; 

        header_.free_page_head_ = page_id; 
        header_.free_page_tail_ = page_id; 
    } else {

        page_header->magic_number = GATORDB_MAGIC_NUMBER; 
        page_header->prev_page_id = header_.free_page_tail_; 
        page_header->next_page_id = INVALID_PAGE_ID; 
        page_header->lower_bound = sizeof(PageHeader);
        page_header->upper_bound = PAGE_SIZE; 
        page_header->slot_count = 0; 
        page_header->deleted_count = 0; 

        header_.free_page_tail_ = page_id; 
    }

    //header_.page_count_--; 
    flush_header(); 

    std::memset(page_buffer.data() + sizeof(PageHeader), '0', PAGE_SIZE - sizeof(PageHeader)); 
    write_page(page_id, page_buffer.data()); 
}

 std::vector<char> Pager::read_page(page_id_t page_id) {
    std::vector<char> buffer(PAGE_SIZE, 0);
    bool success = disk_manager_->read_page(page_id, buffer.data());
    if (!success) {
        throw std::runtime_error("Failed to read page");
    }
    return buffer;  // Moved, not copied
}

bool Pager::write_page(page_id_t page_id, const char* data) {
    bool success = disk_manager_->write_page(page_id, data); 
    if (!success) {
        throw std::runtime_error("Failed to write page to disk.");
    }
    return success; 
}


uint16_t Pager::insert_record(std::vector<char> record, page_id_t page_id) {

    std::vector<char> page_buffer = read_page(page_id); 
    PageHeader* page_header = reinterpret_cast<PageHeader*>(page_buffer.data()); 
    Slot* slots = reinterpret_cast<Slot*>(page_buffer.data() + sizeof(PageHeader)); 

    uint16_t record_length = record.size(); 
    uint16_t required_space = record_length + sizeof(Slot); 

    if (page_header->get_free_space() < required_space) {
        return -1; 
    }

     for (uint16_t i = 0; i < page_header->slot_count; ++i) {
        if (slots[i].length < 0 && std::abs(slots[i].length) >= record_length) {
            uint16_t offset = slots[i].offset;
            std::memcpy(page_buffer.data() + offset, record.data(), record_length); 
            slots[i].length = record_length; // Update the slot to reflect the new record length
            //free space stays unchanged
            return i; // Return the slot number where the record was inserted
        }
    }

    uint16_t offset = page_header->upper_bound - record_length; 
    std::memcpy(page_buffer.data() + offset, record.data(), record_length);
    // Update the slot directory
    uint16_t new_slot_num = page_header->slot_count;
    slots[new_slot_num].offset = offset; 
    slots[new_slot_num].length = record_length;
    // Update the page header
    page_header->upper_bound = offset;
    page_header->lower_bound += sizeof(Slot);
    page_header->slot_count++; 

    write_page(page_id, page_buffer.data()); 
    return new_slot_num; // Return the slot number where the record was inserted
}

std::vector<char> Pager::get_record_raw_bytes(page_id_t page_id, uint16_t slot_num) {
    std::vector<char> page_buffer = read_page(page_id); 
    PageHeader* page_header = reinterpret_cast<PageHeader*>(page_buffer.data()); 
    if (slot_num > page_header->slot_count) return {}; 
    Slot* slots = reinterpret_cast<Slot*>(page_buffer.data() + sizeof(PageHeader)); 
    Slot* target_slot = &slots[slot_num]; 
    if (target_slot == nullptr || target_slot->length < 0) { // Invalid slot number or deleted record
        //std::cout << "Error: Record not found or has been deleted. " << std::endl;
        return {}; 
    }
    std::vector<char> result(page_buffer.begin() + target_slot->offset, page_buffer.begin() + target_slot->offset + target_slot->offset); 
    return result; 

}

bool Pager::delete_record(page_id_t page_id, uint16_t slot_num) {
    std::vector<char> page_buffer = read_page(page_id); 
    PageHeader* page_header = reinterpret_cast<PageHeader*>(page_buffer.data()); 
    if (slot_num > page_header->slot_count) return false; 
    Slot* slots = reinterpret_cast<Slot*>(page_buffer.data() + sizeof(PageHeader)); //get_slot()
    if (slots == nullptr || slots->length < 0) { // Invalid slot number or already deleted
        return false;
    }
    Slot* target_slot = &slots[slot_num]; 
    (target_slot)->length = -(target_slot)->length; 
    //page_header->slot_count--; 
    page_header->deleted_count++; //Currently no way to trigger deallocate_page
    write_page(page_id, page_buffer.data());
    return true; 

}

// PageHeader* Pager::get_page_header(page_id_t page_id) {
//     std::vector<char> page_buffer = read_page(page_id); 
//     PageHeader* page_header = reinterpret_cast<PageHeader*>(page_buffer.data()); 
// }