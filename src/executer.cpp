#include "executer.h"
#include <string> 

inline bool Matches(const std::vector<Value>& row, const Predicate& pred, const size_t pred_col_idx ) {

        if (!pred.Active() || pred_col_idx >= row.size()) return true;
        const Value& v = row[pred_col_idx];
        if (v.GetType() == Type::INTEGER) {
            int32_t rowVal = v.GetData().integer_;
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
            std::string rowStr = v.GetData().string_;
            if (pred.op == WhereOp::EQ)  return rowStr == pred.value;
            if (pred.op == WhereOp::NEQ) return rowStr != pred.value;
        }
        return true;
    };

void Executer::ExecuteCommand(const Statement& stmt, Pager& pager, Catalog& catalog){
    switch (stmt.GetStatementType()) {
        case Statement::STATEMENT_CREATE: 
            ExecuteCreate(stmt, catalog); 
            break; 
        case Statement::STATEMENT_DROP: 
            ExecuteDrop(stmt, catalog); 
            break; 
        case Statement::STATEMENT_INSERT: 
            ExecuteInsert(stmt, pager, catalog); 
            break; 
        case Statement::STATEMENT_SELECT: 
            ExecuteSelect(stmt, pager, catalog);
            break;
        case Statement::STATEMENT_DELETE: 
            ExecuteDelete(stmt, pager, catalog);
            break;
        case Statement::STATEMENT_PEEK: 
            ExecutePeek(stmt, pager); 
            break; 
        case Statement::STATEMENT_EXIT:  
            std::cout << "Exiting..." << std::endl;
            exit(0);
        default:
            std::cout << "Unknown statement type!" << std::endl; 
            
    }
}

void Executer::ExecuteCreate(const Statement& stmt, Catalog& catalog){
    std::cout<< "Executing CREATE statement..." << std::endl; 
    
    std::string table_name = stmt.GetTableName(); 
    Schema schema = stmt.GetSchema(); 
    catalog.CreateTable(table_name, schema); 

}

void Executer::ExecuteDrop(const Statement& stmt, Catalog& catalog){
    std::cout<< "Executing DROP statement..." << std::endl; 
    
    std::string table_name = stmt.GetTableName(); 
    catalog.DropTable(table_name); 

}


void Executer::ExecuteInsert(const Statement& stmt, Pager& pager, const Catalog& catalog) {
    std::cout<< "Executing INSERT statement..." << std::endl;

    std::string  table_name = stmt.GetTableName(); 
    std::vector<std::string> new_records = stmt.GetTargetValues(); 
    auto table_map = catalog.GetTables(); 

    if (table_map.find(table_name) == table_map.end()) {
        std::cout << "Insert Failed. The table does not exist." << std::endl; 
        return;
    }
    Schema schema = table_map.find(table_name)->second; 
    //Need to validate input
    WriteRecord(pager, table_name, schema, new_records); 

}


void Executer::ExecuteSelect(const Statement& stmt, const Pager& pager, const Catalog& catalog) { 
    std::cout << "Executing SELECT statement..." << std::endl; 

    std::string table_name = stmt.GetTableName(); 
    std::string column_name = stmt.GetColumnName(); // "*" or a specific column name
    std::unordered_map<std::string, Schema> table_map = catalog.GetTables(); 

    if (table_map.find(table_name) == table_map.end()) {
        std::cout << "Select failed. The table does not exist." << std::endl; 
        return;
    }

    Schema schema = table_map.find(table_name)->second; 
    const std::vector<Column>& columns = schema.GetColumns();

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

    std::vector<std::vector<Value>> records = ReadTable(pager, table_name); 

    //Where clause 
    Predicate pred = stmt.GetPredicate();

    // Find the column index the predicate targets
    size_t pred_col_idx = SIZE_MAX; 
    if (pred.Active()) {
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i].name == pred.column) { pred_col_idx = i; break; }
        }
    }

    // Print header
    std::cout << table_name << std::endl; 
    for (size_t i : col_indices) {
        std::string col_type = (columns[i].type == Type::VARCHAR) ? "VARCHAR" : "INT"; 
        std::cout << columns[i].name << " " << col_type << columns[i].length << "  |  "; 
    }
    std::cout << std::endl; 

    // Print rows — only the projected columns
    for (const std::vector<Value>& row : records) {
        if (!Matches(row, pred, pred_col_idx)) continue;  // <-- filter here
        for (size_t i : col_indices) {
            if (i >= row.size()) continue; //safety check
            std::string data; 
            if (row[i].GetType() == Type::INTEGER) {
                data = std::to_string(row[i].GetData().integer_); 
            } else if (row[i].GetType() == Type::VARCHAR) {
                data = row[i].GetData().string_; 
            }
            std::cout << data << "  |  "; 
        }
        std::cout << std::endl; 
    }
}

void Executer::ExecuteDelete(const Statement& stmt, Pager& pager, const Catalog& catalog) {
    std::cout << "Executing DELETE statement..." << std::endl;

    std::string table_name = stmt.GetTableName(); 
    std::unordered_map<std::string, Schema> table_map = catalog.GetTables(); 

    if (table_map.find(table_name) == table_map.end()) {
        std::cout << "Select failed. The table does not exist." << std::endl; 
        return;
    }

    Schema schema = table_map.find(table_name)->second; 
    const std::vector<Column>& columns = schema.GetColumns();

    if (table_map.find(table_name) == table_map.end()) {
        std::cout << "Delete failed. The table does not exist." << std::endl; 
        return;
    }

    Predicate pred = stmt.GetPredicate(); 
    std::vector<std::vector<Value>> records = ReadTable(pager, table_name); 

    size_t pred_col_idx = SIZE_MAX; 
    if (pred.Active()) {
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i].name == pred.column) { pred_col_idx = i; break; }
        }
    }

    for (std::vector<Value>& row : records) {
        if (!Matches(row, pred, pred_col_idx)) continue;  // <-- filter here
        //rows_to_delete.push_back(row); 
        DeleteRow(pager, row, table_name); 
    }

}

void Executer::ExecutePeek(const Statement& stmt, const Pager& pager) {

    std::cout << "Executing PEEK statement..." << std::endl; 
    page_id_t page_id = stmt.GetRid().page_id; 

    PeekPage(pager, page_id); 
 
}
