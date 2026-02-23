#pragma once

#include "pager.h"
#include <unordered_map>
#include <vector>
#include <string>

//Internal helpers: 
//Pack & unpack table metadata
std::vector<char> Serialize(TableMetadata& metadata);
TableMetadata Deserialize(const std::vector<char> &buffer); 
//Pack & unpack record data
std::vector<char> Serialize(const Schema& schema, const std::vector<std::string>& values); 
Value Deserialize(const char* buffer, Type type); 
//data operations
page_id_t InsertAtPageOrNew(Pager& pager, page_id_t page_head, const std::vector<char>& raw_bytes); 
std::vector<Value> ReadRow(Schema& schema, const std::vector<char>& raw_bytes); 

//Catalog calls below: 
//Table Metadata
void WriteTableMetadata(Pager& pager, const std::string& table_name, const Schema& schema); 
// Catalog callS it during init
std::vector<TableMetadata> ReadAllTableMetadata(const Pager& pager); 
void DeleteTableMetadata(Pager& pager, const std::string& table_name); 
//Records
void TruncateTable(Pager& pager, const std::string& table_name); 

//Executor calls below: 
void WriteRecord(Pager& pager, std::string& table_name, Schema& schema, const std::vector<std::string>& records); 
//Get all records from table
std::vector<std::vector<Value>> ReadTable(const Pager& pager, const std::string& table_name); 
void PeekPage(const Pager& pager, page_id_t page_id); 
void DeleteRow(const Pager& pager, std::vector<Value>& target_row, const std::string& table_name); 

