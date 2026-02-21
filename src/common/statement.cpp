#include "statement.h" 
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <memory>

//2.0 

void parse_type_and_length(std::string raw, Type& type, uint32_t& length, uint32_t& offset) {
    std::string typeStr;
    std::string lenStr;

    for (char c : raw) {
        if (std::isalpha(c)) typeStr += std::toupper(c);
        else if (std::isdigit(c)) lenStr += c;
    }

    if (typeStr == "VARCHAR") type = Type::VARCHAR;
    else if (typeStr == "INT") type = Type::INTEGER; 
    else type = Type::UNKNOWN;

    length = lenStr.empty() ? 0 : std::stoul(lenStr);
}


std::unique_ptr<StatementParser> StatementParser::parse_statement(const std::string& input_line_) {
    auto stmt = std::unique_ptr<StatementParser>(new StatementParser());
    std::vector<Column> columns; 
    std::stringstream ss(input_line_); 
    std::string command; 
    ss >> command; 

    if (str_to_upper(command) == "CREATE") {
        stmt->type_ = STATEMENT_CREATE; 

        ss >> command; 
        ss >> stmt->table_name_; 

        uint32_t offset = 0; 

        while (ss >> command) {
            Column col; 
            col.name = command;

            if (ss >> command) {
                parse_type_and_length(command, col.type, col.length, col.offset);
            }

            if (col.type == Type::INTEGER) { col.length = sizeof(uint32_t); } 

            col.offset = offset; 
            offset += col.length; 
            columns.push_back(col); 
        }
        stmt->schema_ = Schema(columns); 
        return stmt; 
    }
    else if (str_to_upper(command) == "DROP") {
        stmt->type_ = STATEMENT_DROP; 

        std::string table_name; 
        if (ss >> table_name) {
            stmt->table_name_ = table_name; 
            return stmt; 
        } else {
            std::cerr << "Error: Invalid syntax for " << command << ". Expected: " << command << " table name. " << std::endl; 
            return nullptr; 
        }
    }
    else if (str_to_upper(command) == "INSERT") {
        stmt->type_ = STATEMENT_INSERT; 

        ss >> stmt->table_name_; 
        if (stmt->table_name_.empty()) return nullptr; 
    
        std::string temp_value; 
        while (ss >> temp_value) {
            stmt->values_.push_back(temp_value); 
        }
        return stmt->values_.empty() ? nullptr : std::move(stmt);
    }
    else if (str_to_upper(command) == "SELECT") {
        stmt->type_ = STATEMENT_SELECT; 

        std::string column, from, table_name; 
        if ((ss >> column >> from >> table_name) && str_to_upper(from) == "FROM") {
            stmt->table_name_ = table_name; 
            stmt->column_name_ = column; 

            std::string where_kw;
            if (ss >> where_kw && str_to_upper(where_kw) == "WHERE") {
                std::string op_str;
                if (ss >> stmt->predicate_.column >> op_str >> stmt->predicate_.value) {
                    if      (op_str == "=")  stmt->predicate_.op = WhereOp::EQ;
                    else if (op_str == "!=") stmt->predicate_.op = WhereOp::NEQ;
                    else if (op_str == "<")  stmt->predicate_.op = WhereOp::LT;
                    else if (op_str == ">")  stmt->predicate_.op = WhereOp::GT;
                    else if (op_str == "<=") stmt->predicate_.op = WhereOp::LTE;
                    else if (op_str == ">=") stmt->predicate_.op = WhereOp::GTE;
                } else {
                    std::cerr << "Error: Invalid WHERE syntax. Expected: WHERE column <op> value" << std::endl;
                    return nullptr; 
                }     
            }
            return stmt; 
        } else {
            std::cerr << "Error: Invalid syntax for SELECT. Expected: SELECT col FROM table [WHERE col op val]" << std::endl; 
            return nullptr; 
        }
    }
    else if (str_to_upper(command) == "DELETE") {
        stmt->type_ = STATEMENT_DELETE; 
        std::string table_name; 
        if (ss >> table_name) {
            stmt->table_name_ = table_name; 
            return stmt; 
        } else {
            std::cerr << "Error: Invalid syntax for " << command << ". Expected: " << command << " table name. " << std::endl; 
            return nullptr; 
        }
    }
    else if (str_to_upper(command) == "PEEK") {
        stmt->type_ = STATEMENT_PEEK; 
        page_id_t pid; 
        if (ss >> pid) {
            stmt->target_rid_ = RID(pid, 0);
            return stmt; 
        } else {
            std::cerr << "Error: Invalid syntax for PEEK. Expected: PEEK <page_id>" << std::endl; 
            return nullptr; 
        }
    }
    else if (str_to_upper(command) == "EXIT" || str_to_upper(command) == "QUIT") {
        stmt->type_ = STATEMENT_EXIT; 
        return stmt; 
    } else {
        std::cerr << "Unknown command: " << command << std::endl;
        return nullptr; 
    }
}