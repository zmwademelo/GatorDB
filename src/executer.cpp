#include "executer.h"
#include <string> 


/*
void Executer::execute_command(const StatementParser& stmt, DiskManager& disk){
    std::memset(write_buffer_, 0, PAGE_SIZE); 
    std::memset(read_buffer_, 0, PAGE_SIZE); 
    switch (stmt.get_statement_type()) {
        case StatementParser::STATEMENT_SELECT:
            std::cout << "Executing SELECT statement..." << std::endl;
            disk.read_page(stmt.get_target_page_id(), this->read_buffer_);
            std::cout << "Read buffer: " << this->read_buffer_ << std::endl;
            break;
        case StatementParser::STATEMENT_INSERT:
            std::cout << "Executing INSERT statement..." << std::endl;
            std::memcpy(write_buffer_, stmt.get_data_buffer().c_str(), stmt.get_data_buffer().length()); //
            disk.write_page(stmt.get_target_page_id(), write_buffer_); 
            std::cout << "Inserted " << stmt.get_data_buffer().c_str() << " with length " << stmt.get_data_buffer().length() << std::endl;
            break;
        case StatementParser::STATEMENT_DELETE:
            std::cout << "Executing DELETE statement..." << std::endl;
            // For simplicity, we can just write an empty page to simulate deletion
            std::memset(write_buffer_, 0, PAGE_SIZE);
            disk.write_page(stmt.get_target_page_id(), write_buffer_);
            break;
        case StatementParser::STATEMENT_EXIT: 
            std::cout << "Exiting..." << std::endl;
            exit(0);
        default:
            std::cout << "Unknown statement type!" << std::endl;  
    }
}; 
*/

void Executer::execute_command(const StatementParser& stmt, DiskManager& disk){
    switch (stmt.get_statement_type()) {
        case StatementParser::STATEMENT_INSERT: 
            execute_insert(stmt, disk); 
            break; 
        case StatementParser::STATEMENT_SELECT: 
            execute_select(stmt, disk);
            break;
        case StatementParser::STATEMENT_DELETE: 
            execute_delete(stmt, disk);
            break;
        case StatementParser::STATEMENT_EXIT:  
            std::cout << "Exiting..." << std::endl;
            exit(0);
        default:
            std::cout << "Unknown statement type!" << std::endl; 
            
    }
}

void Executer::execute_insert(const StatementParser& stmt, DiskManager& disk) {
    std::cout<< "Executing INSERT statement..." << std::endl;
    Pager pager = Pager(&disk);
    page_id_t target_page_id = pager.get_available_page_id(stmt.get_data_buffer().length() + sizeof(TablePage::Slot));
    if (target_page_id == static_cast<page_id_t>(-1)) {
        std::cout << "Error: No available page with enough space for the record." << std::endl;
        return; 
    }
    char page_buffer[PAGE_SIZE] = {0};
    pager.read_page(target_page_id, page_buffer);
    TablePage page(page_buffer);
    uint16_t slot_num = page.insert_record(stmt.get_data_buffer().c_str());
    if (slot_num == static_cast<uint16_t>(-1)) {
        std::cout << "Error: Failed to insert record into page." << std::endl;
        return;
    }
    pager.write_page(target_page_id, page_buffer);
    pager.increment_global_record_count();
    std::cout << "Inserted record into page " << target_page_id << ", slot " << slot_num << "." << std::endl;
    return; 
}        

void Executer::execute_select(const StatementParser& stmt, DiskManager& disk) { 
    const RID &target_rid = stmt.get_rid(); 
    std::cout << "Executing SELECT statement for RID (Page ID: " << target_rid.page_id << ", Slot Num: " << target_rid.slot_num << ")..." << std::endl; 
    Pager pager = Pager(&disk); 
    char page_buffer[PAGE_SIZE] = {0}; 
    pager.read_page(target_rid.page_id, page_buffer);
    TablePage page(page_buffer); 
    std::string record = page.get_record(target_rid.slot_num); 
    if (record == "") {
        //std::cout << "Error: Record not found or has been deleted. " << std::endl;
        return; 
    }
    std::cout << "Record at Page " << target_rid.page_id << ", Slot " << target_rid.slot_num << ": " << record << std::endl; 
    return; 

}

void Executer::execute_delete(const StatementParser& stmt, DiskManager& disk) {
    const RID &target_rid = stmt.get_rid(); 
    std::cout << "Executing DELETE statement for RID (Page ID: " << target_rid.page_id << ", Slot: " << target_rid.slot_num << ")..." << std::endl; 
    Pager pager = Pager(&disk); 
    char page_buffer[PAGE_SIZE] = {0}; 
    pager.read_page(target_rid.page_id, page_buffer);
    TablePage page(page_buffer); 
    bool success = page.delete_record(target_rid.slot_num); 
    if (!success) {
        std::cout << "Error: Record not found or already deleted." << std::endl; 
        return; 
    }
    pager.write_page(target_rid.page_id, page_buffer);
    pager.decrement_global_record_count();
    std::cout << "Record at Page " << target_rid.page_id << ", Slot " << target_rid.slot_num << ":  has been deleted. " << std::endl; 
    return; 
}