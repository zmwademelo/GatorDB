#include "statement.h" 
#include <string>

std::string StatementParser::str_to_upper(const std::string& str) const {
    std::string upper_str = str;
    for (char& c : upper_str) { //unsiged? 
        c = toupper(c);
    }
    return upper_str;
}



StatementParser::statement_type StatementParser::GetStatementType() const{ 
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

std::string StatementParser::GetDataBuffer() const {
    if (this->type_ != STATEMENT_INSERT) {
        return "";
    }
    size_t first_space = this->input_line_.find(' ');
    if (first_space == std::string::npos) { //npos is of type size_t and means no position found
        return "";
    }
    auto second_space = this->input_line_.find(' ', first_space + 1);
    if (second_space == std::string::npos) {
        return "";
    }
    return this->input_line_.substr(second_space + 1);
}

size_t StatementParser::GetTargetPageId() const{
    if (str_to_upper(this->input_line_).substr(0, 6) != "INSERT" && str_to_upper(this->input_line_).substr(0, 6) != "SELECT" && str_to_upper(this->input_line_).substr(0, 6) != "DELETE") {
        return 0; 
    }
    size_t first_space = this->input_line_.find(' '); 
    auto second_space = this->input_line_.find(' ', first_space + 1); 
    return std::stoull(this->input_line_.substr(first_space + 1, second_space - first_space - 1));
}