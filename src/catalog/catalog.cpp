
#include "catalog.h"

#include <iostream>
#include <cstring>

void Catalog::CreateTable(const std::string& table_name, const Schema& schema) {
    if (tables_.find(table_name) != tables_.end()) {
        std::cout << "Create Table Failed. The table name already exists." << std::endl; 
        return;
    }
    tables_.emplace(table_name, schema); 
    //Persist
    WriteTableMetadata(*pager_, table_name, schema); 

    std::cout << "Created Table " << table_name << ". All table list: " << std::endl; 
    for (auto item : tables_) {
        std::cout << item.first << " "; 
        for (auto col : item.second.GetColumns()) {
            std::string col_type = (col.type == Type::VARCHAR)? "VARCHAR" : "INT"; 
            std::cout << col.name << " " << col_type << " " << col.length << " | "; 
        }
        std::cout << std::endl;
    }
    return; 
}

void Catalog::DropTable(const std::string& table_name) {
    if (tables_.find(table_name) == tables_.end()) {
        std::cout << "Drop Table Failed. The table does not exist." << std::endl; 
        return;
    }

    tables_.erase(table_name); 
    //Delete table data and persist
    TruncateTable(*pager_, table_name); 
    DeleteTableMetadata(*pager_, table_name); 
    
}



