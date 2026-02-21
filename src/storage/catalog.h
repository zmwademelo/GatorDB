#pragma once

#include "common/config.h"
#include "schema.h" 
#include "pager.h"
#include "storage_utils.h"

#include <string>
#include <vector>
#include <unordered_map>




class Catalog {
    private: 
    std::unordered_map<std::string, Schema> tables_ = {}; 
    Pager* pager_; 

    public: 
    //pager_ must outlive this
    Catalog(Pager* pager) : pager_(pager) {
        for (auto metadata : readAllTableMetadata(*pager_)) {
            tables_.emplace(metadata.table_name, metadata.schema); 
        }
    }


    std::unordered_map<std::string, Schema> get_tables() {return tables_; } 
    Pager& get_pager() { return *pager_; }

    void create_table(std::string& table_name, const Schema& schema); 
    void drop_table(std::string& table_name); 
    void insert_record(std::string& table_name, const std::vector<std::string> records); 

}; 

