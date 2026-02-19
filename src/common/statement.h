#pragma once

#include "common/rid.h"
#include "storage/tuple.h"
#include <string> 
#include <vector> 



//2.0 
class StatementParser {
    public: 
    //Type of statements
    enum statement_type {
        STATEMENT_CREATE, 
        STATEMENT_DROP, 
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
    const Schema& get_schema() const {return schema_; }; 
    const std::string get_table_name() const {return table_name_; }; 
    const std::string& get_data_buffer() const {return data_buffer_;};

    const std::vector<std::string> get_target_values() const { return values_;}  

    private: 
        statement_type type_;
        RID target_rid_;
        std::string data_buffer_; 
        Schema schema_; 
        std::string table_name_; 
        std::vector<std::string> values_; //values of columns

        std::string str_to_upper(const std::string& str) const;

}; 

