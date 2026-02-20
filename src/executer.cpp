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

    std::string table_name = stmt.get_table_name(); 
    std::string column_name = stmt.get_column_name(); // "*" or a specific column name
    std::unordered_map<std::string, Schema> table_map = catalog_.get_tables(); 

    if (table_map.find(table_name) == table_map.end()) {
        std::cout << "Select failed. The table does not exist." << std::endl; 
        return;
    }

    Schema schema = table_map.find(table_name)->second; 
    const std::vector<Column>& columns = schema.get_columns();

    // Build the list of column indices to project
    std::vector<size_t> col_indices;
    if (column_name == "*") {
        for (size_t i = 0; i < columns.size(); ++i)
            col_indices.push_back(i);
    } else {
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i].name == column_name) {
                col_indices.push_back(i);
                break;
            }
        }
        if (col_indices.empty()) {
            std::cout << "Select failed. Column '" << column_name
                      << "' does not exist in table '" << table_name << "'." << std::endl;
            return;
        }
    }

    std::vector<std::vector<Value>> records = readRecords(pager_, table_name); 

    //Where clause 
    Predicate pred = stmt.get_predicate();

    // Find the column index the predicate targets
    size_t pred_col_idx = SIZE_MAX; 
    if (pred.active()) {
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i].name == pred.column) { pred_col_idx = i; break; }
        }
    }

    // Helper lambda to evaluate one row against the predicate
    auto matches = [&](const std::vector<Value>& row) -> bool {
        if (!pred.active() || pred_col_idx >= row.size()) return true;
        const Value& v = row[pred_col_idx];
        if (v.get_type() == Type::INTEGER) {
            int32_t rowVal = v.get_data().integer_;
            int32_t cmpVal = std::stoi(pred.value);
            switch (pred.op) {
                case WhereOp::EQ:  return rowVal == cmpVal;
                case WhereOp::NEQ: return rowVal != cmpVal;
                case WhereOp::LT:  return rowVal <  cmpVal;
                case WhereOp::GT:  return rowVal >  cmpVal;
                case WhereOp::LTE: return rowVal <= cmpVal;
                case WhereOp::GTE: return rowVal >= cmpVal;
                default: return true;
            }
        } else { // VARCHAR
            std::string rowStr = v.get_data().string_;
            if (pred.op == WhereOp::EQ)  return rowStr == pred.value;
            if (pred.op == WhereOp::NEQ) return rowStr != pred.value;
        }
        return true;
    };
    // Print header
    std::cout << table_name << std::endl; 
    for (size_t i : col_indices) {
        std::string col_type = (columns[i].type == Type::VARCHAR) ? "VARCHAR" : "INT"; 
        std::cout << columns[i].name << " " << col_type << columns[i].length << "  |  "; 
    }
    std::cout << std::endl; 

    // Print rows — only the projected columns
    for (const std::vector<Value>& row : records) {
        if (!matches(row)) continue;  // <-- filter here
        for (size_t i : col_indices) {
            if (i >= row.size()) continue; //safety check
            std::string data; 
            if (row[i].get_type() == Type::INTEGER) {
                data = std::to_string(row[i].get_data().integer_); 
            } else if (row[i].get_type() == Type::VARCHAR) {
                data = row[i].get_data().string_; 
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

    truncateTable(pager_, table_name); //delete from
}

void Executer::execute_peek(const StatementParser& stmt) {

    std::cout << "Executing PEEK statement..." << std::endl; 
    page_id_t page_id = stmt.get_rid().page_id; 

    peekPage(pager_, page_id); 
 
}
