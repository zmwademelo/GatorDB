#pragma once


#include "pager.h"
#include <unordered_map>
#include <vector>
#include <string>


//Helpers: 
//Matedata
std::vector<char> serialize(TableMetadata& metadata);
std::vector<char> serialize(TableMetadata& metadata);

//Record data
std::vector<char> serialize(const Schema& schema, const std::vector<std::string>& values); 
Value deserialize(const char* buffer, Type type); 

page_id_t insert_at_page_or_new(Pager& pager, page_id_t page_head, const std::vector<char>& raw_bytes); 

// Catalog calls it
void writeTableMetadata(Pager& pager, std::string& table_name, const Schema& schema); 

// Catalog callS it during init
std::vector<TableMetadata> readAllTableMetadata(Pager& pager); 

void deleteTableMetadata(Pager& pager, std::string& table_name); 

void truncateTable(Pager& pager, std::string& table_name); 

// Executor calls it
void writeRecord(Pager& pager, std::string& table_name, Schema& schema, const std::vector<std::string>& records); 

// Executor calls it
//Get all records from table
std::vector<std::vector<Value>> readRecords(Pager& pager, const std::string& table_name); 

std::vector<Value> readRow(Schema& schema, const std::vector<char>& raw_bytes); 

void peekPage(Pager& pager, page_id_t page_id); 

