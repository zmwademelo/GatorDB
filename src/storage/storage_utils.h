#pragma once


#include "pager.h"
#include <unordered_map>
#include <vector>
#include <string>


//Helpers: 
std::vector<char> serialize(TableMetadata& metadata);
page_id_t try_insert_at_page(Pager& pager, page_id_t page_head, std::vector<char>& raw_bytes); 
// Catalog calls it
void writeTableMetadata(Pager& pager, std::string table_name, const Schema& schema); 

// Catalog callS it during init
std::vector<TableMetadata> readAllTableMetadata(Pager& pager); 

void deleteTableMetadata(Pager& pager, std::string table_name); 

void truncateTable(Pager& pager, std::string table_name); 



// Executor calls it
void writeRecord(Pager& pager, std::string& table_name, Schema& schema, std::vector<std::string>& records); 

// Executor calls it
std::vector<std::vector<Value>> readRecords(Pager& pager, const std::string& table_name); 

std::vector<Value> readRow(Schema& schema, std::vector<char> raw_bytes); 

void peekPage(Pager& pager, page_id_t page_id); 

