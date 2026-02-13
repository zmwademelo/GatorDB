#include "executer.h"
#include <string> 

Schema Executer::initilize_default_schema() {
    std::vector<Column> cols = {
        {"id", Type::INTERGER, 4, 0},     // 4 bytes for int
        {"name", Type::VARCHAR, 20, 0},   // 20 bytes for name
        {"major", Type::INTERGER, 4, 0}   // 10 bytes for major
    };
    
    // The Schema constructor will automatically calculate the correct offsets
    return Schema(cols);
}

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
        case StatementParser::STATEMENT_PEEK: 
            execute_peek(stmt, disk); 
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
    /* For Player only
    Tuple::Player player = stmt.get_target_player();

    // 2. Turn into bytes
    std::vector<char> bytes = Tuple::serialize(player);
    std::string binary_str(bytes.begin(), bytes.end()); 

    */
    //genaral Schema
    const std::vector<std::string> raw_values = stmt.get_target_values(); 

    if (raw_values.size() != current_schema_.get_columns().size()) {
        std::cout << "Error: Column count mismatch." << std::endl;
        return;
    }

    std::vector<char> binary_data = Tuple::serialize(current_schema_, raw_values); //?Shouldn't is use reference? 
    uint32_t record_size = current_schema_.get_designed_size(); 


    Pager pager = Pager(&disk);
    //Player //page_id_t target_page_id = pager.get_available_page_id(binary_str.length() + sizeof(TablePage::Slot));
    //page_id_t target_page_id = pager.get_available_page_id(stmt.get_data_buffer().length() + sizeof(TablePage::Slot));
    page_id_t target_page_id = pager.get_available_page_id(record_size + sizeof(TablePage::Slot));
    if (target_page_id == static_cast<page_id_t>(-1)) {
        std::cout << "Error: No available page with enough space for the record." << std::endl;
        return; 
    }
    char page_buffer[PAGE_SIZE] = {0};
    pager.read_page(target_page_id, page_buffer);
    TablePage page(page_buffer);
    uint16_t slot_num = page.insert_record(binary_data.data(), record_size);
    //uint16_t slot_num = page.insert_record(binary_str.c_str(), binary_str.length());
    //uint16_t slot_num = page.insert_record(stmt.get_data_buffer().c_str());
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
    /*
    //Player 
    Tuple::Player player = Tuple::deserailize(record); 
    std::cout << "Record at Page " << target_rid.page_id << ", Slot " << target_rid.slot_num << ": " 
    << std::endl << "Name: " << player.name << "\nYOB: " << player.yob << "\nMajor: " << player.major << std::endl; 
    */
    std::vector<std::string> row = Tuple::deserialize(current_schema_, record); 

    for(size_t i = 0; i < row.size(); ++i) {
        std::cout << current_schema_.get_columns()[i].name << ": " << row[i] << " | "; 
    }
    std::cout << std::endl; 
   //std::cout << "Record at Page " << target_rid.page_id << ", Slot " << target_rid.slot_num << ": " << record << std::endl; 
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

void Executer::execute_peek(const StatementParser& stmt, DiskManager& disk){
    std::cout << "Peeking..." <<std::endl; 
    const RID &target_rid = stmt.get_rid(); 
    Pager pager = Pager(&disk); 
    FileHeader* file_header = pager.get_file_header(); 
    if (target_rid.page_id == 0){ //For File header
        std::cout<< "------File Header: ------" << std::endl;  
        std::cout <<"Magic Number: " << file_header->magic_number_ << std::endl; 
        std::cout <<"Next Page ID: " << file_header->next_page_id << std::endl; 
        std::cout <<"Record Count: " << file_header->record_count << std::endl; 
        return; 
    }
    if (target_rid.page_id >= file_header->next_page_id) {
        std::cout<< "Page has not been allocated yet. " << std::endl;  
        return; 
    }
    char page_buffer[PAGE_SIZE] = {0}; 
    pager.read_page(target_rid.page_id, page_buffer); 
    TablePage page(page_buffer); 
    std::cout<< "------Page Header: ------" << std::endl; 
    //auto header_start = reinterpret_cast<TablePage::PageHeader*>(page_buffer); 
    std::cout <<"Magic Number: " << page.get_page_header()->magic_number << std::endl; 
    std::cout <<"Upper Bound: " << page.get_page_header()->upper_bound << std::endl;
    std::cout <<"Lower Bound: " << page.get_page_header()->lower_bound << std::endl;
    std::cout <<"Slot Count: " << page.get_page_header()->slot_count << std::endl;
    std::cout <<"Free Space: " << page.get_page_header()->free_space << std::endl; 
    
    std::cout<< "------Page Data: ------" << std::endl; 
    
    for (auto i = 0; i < page.get_slot_count(); ++i) {
        TablePage::Slot* slot = page.get_slot(i); 
        
        if (slot != nullptr && slot->length > 0)
        {
            /*
            //Player 
            Tuple::Player player = Tuple::deserailize(page.get_record(i)); 
            std::cout << "Record at Page " << target_rid.page_id << ", Slot " << i << ": " 
            << std::endl << "Name: " << player.name << "\nYOB: " << player.yob << "\nMajor: " << player.major << std::endl; 
            */

            //Schema
            std::vector<std::string> row = Tuple::deserialize(current_schema_, page.get_record(i)); 
            std::cout << "Record at Page " << target_rid.page_id << ", Slot " << i << ": " << std::endl; 
            for(size_t i = 0; i < row.size(); ++i) {
                std::cout << current_schema_.get_columns()[i].name << ": " << row[i] << " | "; 
            }
             std::cout << std::endl; 


            //std::cout << "Slot[" << i <<"]: (" << slot->length << " Bytes) "<< page.get_record(i) << std::endl; 
        }
        
    }
    
    return; 

}