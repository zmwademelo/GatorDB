#include "pager.h"
#include "table_page.h"

Pager::Pager(std::unique_ptr<DiskManager> diskmanager) : disk_manager_(std::move(diskmanager)) {}

// static
std::unique_ptr<Pager> Pager::Create(const std::string db_file_name)
{
    std::unique_ptr<DiskManager> disk = std::make_unique<DiskManager>(db_file_name);
    //std::unique_ptr<Pager> pager = std::make_unique<Pager>(std::move(disk)); 
    /*
    Create() is the only way to create a Pager object. 
    Cannot use make_unique because the default constructor is private
    */
    std::unique_ptr<Pager> pager(new Pager(std::move(disk)));

    if (!pager->LoadFileHeader())
    {
        pager->InitFileHeader();
        pager->AllocateNewPage();
    }
    return pager;
}

bool Pager::LoadFileHeader()
{
    char buffer[PAGE_SIZE] = {0};
    bool header_exists = disk_manager_->ReadPage(0, buffer) && buffer[0] != '\0';

    if (header_exists) {
        // Existing database file - load and validate header
        std::memcpy(&file_header_, buffer, sizeof(FileHeader));
        if (file_header_.magic_number != GATORDB_MAGIC_NUMBER) {
            throw std::runtime_error("Invalid database file: Magic number mismatch.");
        } 
        return true;      
    } else { return false; }
}

void Pager::InitFileHeader()
{
    file_header_ = FileHeader();
    FlushFileHeader();
}

void Pager::FlushFileHeader()
{
    char buffer[PAGE_SIZE] = {0}; // Initialize buffer to zero
    std::memcpy(buffer, &file_header_, sizeof(FileHeader));
    disk_manager_->WritePage(0, buffer);
}

void Pager::InitPage(page_id_t page_id)
{
    if (page_id < 1)
        return;
    char buffer[PAGE_SIZE] = {0};
    PageHeader page_header = PageHeader();
    std::memcpy(buffer, &page_header, sizeof(PageHeader));
    //Persist
    disk_manager_->WritePage(page_id, buffer); 
}

// 2.0

page_id_t Pager::AllocateNewPage()
{
    page_id_t new_page_id;
    char page_buffer[PAGE_SIZE] = {0};
    if (file_header_.free_page_head > 0 && file_header_.free_page_head != INVALID_PAGE_ID)
    { // Recycled
        new_page_id = file_header_.free_page_head;

        std::vector<char> free_page_head_buffer = ReadPage(new_page_id);
        PageHeader &free_page_head_header = *reinterpret_cast<PageHeader *>(free_page_head_buffer.data());

        if (file_header_.free_page_head == file_header_.free_page_tail)
        { // only one free page
            file_header_.free_page_head = INVALID_PAGE_ID;
        }else{
            file_header_.free_page_head = free_page_head_header.next_page_id;
        }
    }
    else
    {
        new_page_id = file_header_.page_count++;
        // When you grow the file, you should ideally write a "blank" page (4096 zeros) to that new offset immediately to "claim" the space.
        InitPage(new_page_id);
    }

    FlushFileHeader();
    return new_page_id;
}

void Pager::DeallocatePage(page_id_t page_id)
{

    if (page_id < 2)
        return; // Page 0 and Page 1 are reserved
    std::vector<char> page_buffer = ReadPage(page_id);
    PageHeader &page_header = *reinterpret_cast<PageHeader *>(page_buffer.data());
    if (file_header_.free_page_head == INVALID_PAGE_ID)
    {
        // Assign free page head first time
        page_header.magic_number = GATORDB_MAGIC_NUMBER;
        page_header.prev_page_id = INVALID_PAGE_ID;
        page_header.next_page_id = INVALID_PAGE_ID;
        page_header.lower_bound = sizeof(PageHeader);
        page_header.upper_bound = PAGE_SIZE;
        page_header.slot_count = 0;
        page_header.deleted_count = 0;

        file_header_.free_page_head = page_id;
        file_header_.free_page_tail = page_id;
    }else{
        // Append this page to the back of free page list
        page_header.magic_number = GATORDB_MAGIC_NUMBER;
        page_header.prev_page_id = file_header_.free_page_tail;
        page_header.next_page_id = INVALID_PAGE_ID;
        page_header.lower_bound = sizeof(PageHeader);
        page_header.upper_bound = PAGE_SIZE;
        page_header.slot_count = 0;
        page_header.deleted_count = 0;

        file_header_.free_page_tail = page_id;
    }

    // header_.page_count_--;
    FlushFileHeader();
    //Reset page content, exlucing the page header
    std::memset(page_buffer.data() + sizeof(PageHeader), '0', PAGE_SIZE - sizeof(PageHeader));
    WritePage(page_id, page_buffer.data());
}

std::vector<char> Pager::ReadPage(page_id_t page_id) const
{
    std::vector<char> buffer(PAGE_SIZE, 0);
    bool success = disk_manager_->ReadPage(page_id, buffer.data());
    if (!success)
    {
        throw std::runtime_error("Failed to read page");
    }
    return buffer; // Moved, not copied
}

bool Pager::WritePage(page_id_t page_id, const char *data) const
{
    bool success = disk_manager_->WritePage(page_id, data);
    if (!success)
    {
        throw std::runtime_error("Failed to write page to disk.");
    }
    return success;
}

uint16_t Pager::InsertRecord(const std::vector<char> &record, page_id_t page_id) const
{

    std::vector<char> page_buffer = ReadPage(page_id);
    PageHeader &page_header = *reinterpret_cast<PageHeader *>(page_buffer.data());
    Slot *slots = reinterpret_cast<Slot *>(page_buffer.data() + sizeof(PageHeader));

    uint16_t record_length = record.size();
    uint16_t required_space = record_length + sizeof(Slot);

    if (page_header.GetFreeSpace() < required_space) { return -1;}

    for (uint16_t i = 0; i < page_header.slot_count; ++i)
    {   //Reuse deleted slot
        if (slots[i].length < 0 && std::abs(slots[i].length) >= record_length) 
        {
            uint16_t offset = slots[i].offset;
            std::memcpy(page_buffer.data() + offset, record.data(), record_length);
            slots[i].length = record_length; // Update the slot to reflect the new record length
            page_header.deleted_count--; 
            WritePage(page_id, page_buffer.data());
            // free space stays unchanged
            return i; // Return the slot number where the record was inserted
        }
    }

    uint16_t offset = page_header.upper_bound - record_length;
    std::memcpy(page_buffer.data() + offset, record.data(), record_length);
    // Update the slot directory
    uint16_t new_slot_num = page_header.slot_count;
    slots[new_slot_num].offset = offset;
    slots[new_slot_num].length = record_length;
    // Update the page header
    page_header.upper_bound = offset;
    page_header.lower_bound += sizeof(Slot);
    page_header.slot_count++;

    WritePage(page_id, page_buffer.data());
    return new_slot_num; // Return the slot number where the record was inserted
}

std::vector<char> Pager::GetRecordRawBytes(page_id_t page_id, uint16_t slot_num) const
{
    std::vector<char> page_buffer = ReadPage(page_id);
    PageHeader &page_header = *reinterpret_cast<PageHeader *>(page_buffer.data());
    if (slot_num > page_header.slot_count)
        return {};
    Slot *slots = reinterpret_cast<Slot *>(page_buffer.data() + sizeof(PageHeader));
    Slot *target_slot = &slots[slot_num];
    //Filters deleted record
    if (target_slot == nullptr || target_slot->length < 0)
    { // Invalid slot number or deleted record
        // std::cout << "Error: Record not found or has been deleted. " << std::endl;
        return {};
    }
    std::vector<char> result(page_buffer.begin() + target_slot->offset, page_buffer.begin() + target_slot->offset + target_slot->offset);
    return result;
}

bool Pager::DeleteRecord(page_id_t page_id, uint16_t slot_num) const
{
    std::vector<char> page_buffer = ReadPage(page_id);
    PageHeader &page_header = *reinterpret_cast<PageHeader *>(page_buffer.data());
    if (slot_num > page_header.slot_count)
        return false;
    Slot *slots = reinterpret_cast<Slot *>(page_buffer.data() + sizeof(PageHeader)); // get_slot()
    if (slots == nullptr)
    { // Invalid slot number or already deleted
        return false;
    }
    Slot *target_slot = &slots[slot_num]; 
    if (target_slot == nullptr || target_slot->length < 0)
    { // Invalid slot number or already deleted
        return false;
    }
    (target_slot)->length = -(target_slot)->length;
    // page_header->slot_count--;
    page_header.deleted_count++; // Currently no way to trigger DeallocatePage
    WritePage(page_id, page_buffer.data());
    return true;
}
