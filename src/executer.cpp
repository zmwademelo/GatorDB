#include "executer.h"
#include <string> 



void Executer::execute_command(const StatementParser& stmt, DiskManager& disk){
    std::memset(write_buffer_, 0, PAGE_SIZE); 
    std::memset(read_buffer_, 0, PAGE_SIZE); 
    switch (stmt.GetStatementType()) {
        case StatementParser::STATEMENT_SELECT:
            std::cout << "Executing SELECT statement..." << std::endl;
            disk.ReadPage(stmt.GetTargetPageId(), this->read_buffer_);
            std::cout << "Read buffer: " << this->read_buffer_ << std::endl;
            break;
        case StatementParser::STATEMENT_INSERT:
            std::cout << "Executing INSERT statement..." << std::endl;
            std::memcpy(write_buffer_, stmt.GetDataBuffer().c_str(), stmt.GetDataBuffer().length()); //
            disk.WritePage(stmt.GetTargetPageId(), write_buffer_); 
            std::cout << "Inserted " << stmt.GetDataBuffer().c_str() << " with length " << stmt.GetDataBuffer().length() << std::endl;
            break;
        case StatementParser::STATEMENT_DELETE:
            std::cout << "Executing DELETE statement..." << std::endl;
            // For simplicity, we can just write an empty page to simulate deletion
            std::memset(write_buffer_, 0, PAGE_SIZE);
            disk.WritePage(stmt.GetTargetPageId(), write_buffer_);
            break;
        case StatementParser::STATEMENT_EXIT: 
            std::cout << "Exiting..." << std::endl;
            exit(0);
        default:
            std::cout << "Unknown statement type!" << std::endl;  
    }
}; 