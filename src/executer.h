#pragma once
#include "storage/catalog.h" 
#include "storage/pager.h"
#include "common/statement.h"
#include "storage/table_page.h"
#include "storage/storage_utils.h"

//#include "storage/table_page.h"
#include <iostream>
#include <string>

class Executer{
    public: 
        Executer(Pager& pager, Catalog& catalog) : pager_(pager), catalog_(catalog) {}
        
        ~Executer() {}
        void execute_command(const StatementParser& stmt); 

        void execute_create(const StatementParser& stmt);
        void execute_drop(const StatementParser& stmt);
        void execute_insert(const StatementParser& stmt); 
        void execute_select(const StatementParser& stmt); 
        void execute_delete(const StatementParser& stmt);
        void execute_peek(const StatementParser& stmt); //For debug
        
        

    private: 
    /*The Executer should receive references to the Pager and the Statement, rather than owning them. */
        
        Pager& pager_; 
        Catalog& catalog_; 
        Schema current_schema_; 
        char read_buffer_[PAGE_SIZE] = {0};
        char write_buffer_[PAGE_SIZE] = {0};
        //Schema current_schema_; 
        //page_id_t current_page_id_; 
        
}; 