#pragma once

#include "common/config.h"
#include "storage/schema.h" 
#include "storage/pager.h"
#include "storage/storage_utils.h"

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


    std::unordered_map<std::string, Schema> get_tables() const {return tables_; } 
    Pager& get_pager() const { return *pager_; }

    void create_table(const std::string& table_name, const Schema& schema); 
    void drop_table(const std::string& table_name); 
    //void insert_record(const std::string& table_name, const std::vector<std::string> records); 

}; 

