#include "statement.h" 
#include <string>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>

/* 1.0
std::string str_to_upper(const std::string& str) {
    std::string upper_str = str;
    for (char& c : upper_str) { //unsiged? 
        c = toupper(c);
    }
    return upper_str;
}
    
std::string trim_leading_spaces(const std::string& str) {
    size_t first_non_space = str.find_first_not_of(' ');
    if (first_non_space != std::string::npos) {
        return str.substr(first_non_space);
    } else {
        return ""; // String is all spaces
    }
}


StatementParser::statement_type StatementParser::get_statement_type() const{ 
    if (str_to_upper(this->input_line_).substr(0, 6) == "INSERT") {
        return STATEMENT_INSERT;
    } else if (str_to_upper(this->input_line_).substr(0, 6) == "SELECT") {
        return STATEMENT_SELECT;
    } else if (str_to_upper(this->input_line_).substr(0, 6) == "DELETE") {
        return STATEMENT_DELETE;
    } else if (str_to_upper(this->input_line_).substr(0, 4) == "QUIT" || str_to_upper(this->input_line_).substr(0, 4) == "EXIT") {
        return STATEMENT_EXIT;
    } else {
        return STATEMENT_UNKNOWN;
    }

}

std::string StatementParser::get_data_buffer() const {
    if (this->type_ != STATEMENT_INSERT) {
        return "";
    }
    size_t first_space = this->input_line_.find(' ');
    if (first_space == std::string::npos) { //npos is of type size_t and means no position found
        return "";
    }
    return this->input_line_.substr(first_space + 1);
}

page_id_t StatementParser::get_target_page_id() const{
    if (str_to_upper(this->input_line_).substr(0, 6) != "INSERT" && str_to_upper(this->input_line_).substr(0, 6) != "SELECT" && str_to_upper(this->input_line_).substr(0, 6) != "DELETE") {
        return 0; 
    }
    auto first_space = this->input_line_.find(' '); 
    if (first_space == std::string::npos) {
        std::cerr<<"Error: No page_id provided!" << std::endl;
        return 0;
    }
    auto second_space = trim_leading_spaces(this->input_line_.substr(first_space + 1)).find(' '); 
    if (second_space == std::string::npos) {
        second_space = this->input_line_.length();
    }
    return std::stoull(this->input_line_.substr(first_space + 1, second_space - first_space - 1));
}

uint16_t StatementParser::get_target_slot_num() const {
    if (str_to_upper(this->input_line_).substr(0, 6) != "INSERT" && str_to_upper(this->input_line_).substr(0, 6) != "SELECT" && str_to_upper(this->input_line_).substr(0, 6) != "DELETE") {
        return 0; 
    }
    auto first_space = this->input_line_.find(' '); 
    if (first_space == std::string::npos) {
        std::cerr<<"Error: No page_id provided!" << std::endl;
        return 0;
    }
    auto second_space = trim_leading_spaces(this->input_line_.substr(first_space + 1)).find(' '); 
    if (second_space == std::string::npos) {
        second_space = this->input_line_.length();
    }
    auto third_space = trim_leading_spaces(this->input_line_.substr(first_space + 1)).find(' ', second_space + 1); 
    if (third_space == std::string::npos) {
        return 0; // No slot number provided, default to 0
    }
    return static_cast<uint16_t>(std::stoul(this->input_line_.substr(second_space + first_space + 2, third_space - second_space - 1)));
}

RID StatementParser::get_target_rid() const {
    // For simplicity, we can return a default RID for now. In a real implementation, you would parse the input to extract the page_id and slot_num for the RID.
    return RID{this->get_target_page_id(), this->target_slot_num_}; 
}

*/
//2.0 

std::string StatementParser::str_to_upper(const std::string& str) const {
    std::string upper_str = str;
    for (char& c : upper_str) { //unsiged? 
        c = toupper(c);
    }
    return upper_str;
}
bool StatementParser::parse_statement(const std::string& input_line_) {
    std::stringstream ss(input_line_); 
    std::string command; 
    ss >> command; 

    if (str_to_upper(command) == "INSERT") {
        type_ = STATEMENT_INSERT; 
        /*
        This is for inserting plain string i.e. insert GATOR
        std::getline(ss >> std::ws, data_buffer_); //ws is for discarding leading whitespace before reading the rest of the line into data_buffer_
        return !data_buffer_.empty(); 
        */
       std::string name; 
       uint16_t yob, major; 
       if (ss >> name >> yob >> major){
        target_player_ = Tuple::Player(name, yob, major);
        return true; 
       }
         else {
          std::cerr << "Error: Invalid syntax for INSERT. Expected: INSERT <name> <yob> <major>" << std::endl;
       return false; 
         }
    }
    else if (str_to_upper(command) == "SELECT" || str_to_upper(command) == "DELETE") {
        type_ = (str_to_upper(command) == "SELECT") ? STATEMENT_SELECT : STATEMENT_DELETE; 

        page_id_t pid;
        uint16_t sid; 
        
        if (ss >> pid >> sid) {
            target_rid_ = RID(pid, sid); 
            return true; 
        } else {
            std::cerr << "Error: Invalid syntax for " << command << ". Expected: " << command << "<page_id> <slot_num>" << std::endl; 
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
    }

    type_ = STATEMENT_UNKNOWN; 
    return false; 
    
}