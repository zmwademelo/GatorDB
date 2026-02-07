#pragma once

#include <string> 

//This class is responsible for parse string from cin to commands
/*
INSERT: insert page_id data_buffer
SELECT: select page_id
DELETE: delete page_id
EXIT: exit
UNKNOWN: unknown command
*/

class StatementParser {
public:
    StatementParser(const std::string& input) 
    : input_line_(input),
      type_(GetStatementType()),
      data_buffer_(GetDataBuffer()),
      target_page_id_(GetTargetPageId()) 
{}
    ~StatementParser() {}; 
    
    enum statement_type {
        STATEMENT_INSERT,
        STATEMENT_SELECT, 
        STATEMENT_DELETE, 
        STATEMENT_EXIT,
        STATEMENT_UNKNOWN}; 
    statement_type GetStatementType() const; 
    std::string GetDataBuffer() const; 
    size_t GetTargetPageId() const; 
private:
    std::string input_line_; 
    std::string str_to_upper(const std::string& str) const;
    statement_type type_; 
    std::string data_buffer_; 
    size_t target_page_id_; 

};
