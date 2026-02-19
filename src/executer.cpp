#include "executer.h"
#include <string> 

//std::vector<char> serialize(const Schema& schema, const std::vector<std::string>& values){

void Executer::execute_command(const StatementParser& stmt){
    switch (stmt.get_statement_type()) {
        case StatementParser::STATEMENT_CREATE: 
            execute_create(stmt); 
            break; 
        case StatementParser::STATEMENT_DROP: 
            execute_drop(stmt); 
            break; 
        case StatementParser::STATEMENT_INSERT: 
            execute_insert(stmt); 
            break; 
        case StatementParser::STATEMENT_SELECT: 
            execute_select(stmt);
            break;
        case StatementParser::STATEMENT_DELETE: 
            execute_delete(stmt);
            break;
        case StatementParser::STATEMENT_PEEK: 
            execute_peek(stmt); 
            break; 
        case StatementParser::STATEMENT_EXIT:  
            std::cout << "Exiting..." << std::endl;
            exit(0);
        default:
            std::cout << "Unknown statement type!" << std::endl; 
            
    }
}

void Executer::execute_create(const StatementParser& stmt){
    std::cout<< "Executing CREATE statement..." << std::endl; 
    
    std::string table_name = stmt.get_table_name(); 
    Schema schema = stmt.get_schema(); 

    catalog_.create_table(table_name, schema); 

}

void Executer::execute_drop(const StatementParser& stmt){
    std::cout<< "Executing DROP statement..." << std::endl; 
    
    std::string table_name = stmt.get_table_name(); 

    catalog_.drop_table(table_name); 

}


void Executer::execute_insert(const StatementParser& stmt) {
    std::cout<< "Executing INSERT statement..." << std::endl;

    std::string  table_name = stmt.get_table_name(); 
    std::vector<std::string> new_records = stmt.get_target_values(); 
    auto table_map = catalog_.get_tables(); 

    if (table_map.find(table_name) == table_map.end()) {
        std::cout << "Insert Failed. The table does not exist." << std::endl; 
        return;
    }
    Schema schema = table_map.find(table_name)->second; 
    //Need to validate input
    writeRecord(pager_, table_name, schema, new_records); 

}



void Executer::execute_select(const StatementParser& stmt) { 
    std::cout << "Executing SELECT statement..." << std::endl; 

    std::string  table_name = stmt.get_table_name(); 
    std::unordered_map<std::string, Schema> table_map = catalog_.get_tables(); 

    if (table_map.find(table_name) == table_map.end()) {
        std::cout << "Select failed. The table does not exist." << std::endl; 
        return;
    }

    Schema schema = table_map.find(table_name)->second; 
    std::vector<std::vector<Value>> records = readRecords(pager_, table_name); 

    std::cout << table_name << std::endl; 
    for (auto col : schema.get_columns()) {
        std::string col_type = (col.type == Type::VARCHAR) ? "VARCHAR" : "INT"; 
        std::cout << col.name << " " << col_type << col.length << "  |  "; 
    }
    std::cout << std::endl; 

    for (auto row : records) {
        for (auto val : row) {
            std::string data; 
            if (val.get_type() == Type::INTEGER) {
                data = std::to_string(val.get_data().integer_); 
            }else if (val.get_type() == Type::VARCHAR) {
                data = val.get_data().string_; 
            } else {
                data = ""; 
            }
            std::cout << data << "  |  "; 
        }
        std::cout << std::endl; 
    }
    


}

void Executer::execute_delete(const StatementParser& stmt) {
    std::cout << "Executing DELETE statement..." << std::endl;

    std::string table_name = stmt.get_table_name(); 
    std::unordered_map<std::string, Schema> table_map = catalog_.get_tables(); 

    if (table_map.find(table_name) == table_map.end()) {
        std::cout << "Delete failed. The table does not exist." << std::endl; 
        return;
    }

    truncateTable(pager_, table_name); 
}

void Executer::execute_peek(const StatementParser& stmt) {

    std::cout << "Executing PEEK statement..." << std::endl; 
    page_id_t page_id = stmt.get_rid().page_id; 

    peekPage(pager_, page_id); 
 
}
