#pragma once

#include "common/rid.h"
#include <string> 

/*Function of Statement class: 
Take a raw string input and parse it into a RID object and a data buffer. 
Pass the RID and data buffer to the Executer, which will then interact with the Pager and Table Page to perform the requested opearation. 
*/

//This class is responsible for parse string from cin to commands
/*
INSERT: insert page_id data_buffer
SELECT: select page_id
DELETE: delete page_id
EXIT: exit
UNKNOWN: unknown command
*/
//This class is responsible for parse string from cin to commands
/* 2.0 ver
INSERT: insert data_buffer
SELECT: select page_id slot_num
DELETE: delete page_id slot_num
EXIT: exit
UNKNOWN: unknown command
*/
/*1.0 
class StatementParser {
public:
    StatementParser(const std::string& input) 
    : input_line_(input),
      type_(get_statement_type()),
      data_buffer_(get_data_buffer()),
      target_page_id_(get_target_page_id()),
      target_slot_num_(get_target_slot_num()), 
      target_rid_(get_target_rid())
{}
    ~StatementParser() {}; 
    
    enum statement_type {
        STATEMENT_INSERT,
        STATEMENT_SELECT, 
        STATEMENT_DELETE, 
        STATEMENT_EXIT,
        STATEMENT_UNKNOWN}; 
    statement_type get_statement_type() const; 
    std::string get_data_buffer() const; 
    RID get_target_rid() const; 
private:
    std::string input_line_; 
    //std::string str_to_upper(const std::string& str) const;
    statement_type type_; 
    std::string data_buffer_; 
    page_id_t target_page_id_; 
    uint16_t target_slot_num_;
    RID target_rid_; 

    page_id_t get_target_page_id() const; 
    uint16_t get_target_slot_num() const;
};
*/

//2.0 
class StatementParser {
    public: 
    enum statement_type {
        STATEMENT_INSERT, 
        STATEMENT_SELECT,
        STATEMENT_DELETE, 
        STATEMENT_PEEK, //For debug
        STATEMENT_EXIT, 
        STATEMENT_UNKNOWN
    }; 

    StatementParser() : type_(STATEMENT_UNKNOWN), target_rid_(0, 0) {}; 

    bool parse_statement(const std::string& input_line_); 

    statement_type get_statement_type() const { return type_; };
    const RID& get_rid() const {return target_rid_; }; 
    const std::string& get_data_buffer() const {return data_buffer_;};


    private: 
        statement_type type_;
        RID target_rid_;
        std::string data_buffer_; 

        std::string str_to_upper(const std::string& str) const;

}; 

