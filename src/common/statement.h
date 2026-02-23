#pragma once

#include "common/rid.h"
#include "storage/tuple.h"
#include <string> 
#include <vector> 

//Used by is a wrong comment pattern, per frog
inline std::string str_to_upper(const std::string& str) {
    std::string upper_str = str;
    for (char& c : upper_str) { //unsiged? 
        c = toupper(c);
    }
    return upper_str;
}
enum class WhereOp { NONE, EQ, NEQ, LT, GT, LTE, GTE };

//Predicate that describes a where condition
struct Predicate {
    std::string column;
    WhereOp op = WhereOp::NONE;
    std::string value;          // stored as string, cast at eval time
    bool active() const { return op != WhereOp::NONE; }
};

//2.0 
class Statement {
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

    static std::unique_ptr<Statement> parse_statement(const std::string& input_line_); //Factory method

    statement_type get_statement_type() const { return type_; }
    const RID& get_rid() const {return target_rid_; }
    const Schema& get_schema() const {return schema_; }
    const std::string get_table_name() const {return table_name_; }
    const std::string get_column_name() const {return column_name_; }
    //const std::string& get_data_buffer() const {return data_buffer_; }
    const std::vector<std::string> get_target_values() const { return values_;}  
    const Predicate& get_predicate() const { return predicate_; }  

    private: 

        Statement() = default; //private constructor

        statement_type type_;
        RID target_rid_; //record page and slot #
        //std::string data_buffer_; 
        Schema schema_; 
        std::string table_name_; 
        std::string column_name_; 
        std::vector<std::string> values_; //values of columns
        Predicate predicate_;

}; 

