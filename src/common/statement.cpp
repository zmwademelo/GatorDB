#include "statement.h" 
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>

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

std::string StatementParser::str_to_upper(const std::string& str) const {
    std::string upper_str = str;
    for (char& c : upper_str) { //unsiged? 
        c = toupper(c);
    }
    return upper_str;
}
bool StatementParser::parse_statement(const std::string& input_line_) {
    std::vector<Column> columns; 
    std::stringstream ss(input_line_); 
    std::string command; 
    ss >> command; 

    if (str_to_upper(command) == "CREATE") {
        type_ = STATEMENT_CREATE; 

        ss >> command; 
        ss >> table_name_; 

        uint32_t offset = 0; 

        while (ss >> command) {
            Column col; 
            col.name = command; // First word is the column name (e.g., NAME)

            if (ss >> command) { // Second word is the type (e.g., VARCHAR30)
                parse_type_and_length(command, col.type, col.length, col.offset);
            }

            if (col.type == Type::INTEGER) {col.length = sizeof(uint32_t);} 

            col.offset = offset; 
            offset += col.length; 
            columns.push_back(col); 
            
        }
        schema_ = Schema(columns); 
        return true; 
    }
    else if (str_to_upper(command) == "DROP") {
        type_ = STATEMENT_DROP; 

        std::string table_name; 
        
        if (ss >> table_name) {
            table_name_ = table_name; 
            return true; 
        } else {
            std::cerr << "Error: Invalid syntax for " << command << ". Expected: " << command << " table name. " << std::endl; 
            return false; 
        }
    }
    else if (str_to_upper(command) == "INSERT") {
        type_ = STATEMENT_INSERT; 
    
        ss >> table_name_; 
        if (table_name_.empty()) return false; 
    
        std::string temp_value; 
        while (ss >> temp_value) {
            values_.push_back(temp_value); 
        }
        return !values_.empty();
    }
    else if (str_to_upper(command) == "SELECT" || str_to_upper(command) == "DELETE") {
        type_ = (str_to_upper(command) == "SELECT") ? STATEMENT_SELECT : STATEMENT_DELETE; 

        std::string table_name; 
        
        if (ss >> table_name) {
            table_name_ = table_name; 
            return true; 
        } else {
            std::cerr << "Error: Invalid syntax for " << command << ". Expected: " << command << " table name. " << std::endl; 
            return false; 
        }
    }
    else if(str_to_upper(command) == "PEEK") {
        type_ = STATEMENT_PEEK; 
        page_id_t pid; 
        if (ss >> pid) {
            target_rid_ = RID(pid, 0); //Iterate from the first slot
            return true; 
        } else {
            std::cerr << "Error: Invalid syntax for " << command << ". Expected: " << command << "<page_id>" << std::endl; 
            return false; 
        }
        return true; 
    }
    else if (str_to_upper(command) == "EXIT" || str_to_upper(command) == "QUIT") {
        type_ = STATEMENT_EXIT; 
        return true; 
    }else{
        type_ = STATEMENT_UNKNOWN; 
        return false; 
    }

    
    
}