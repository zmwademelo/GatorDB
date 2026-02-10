#pragma once
#include "common/statement.h"
#include "storage/disk_manager.h"
#include "storage/pager.h"
#include <iostream>
#include <string>

class Executer{
    public: 
        Executer(const StatementParser& stmt) : parser_(stmt) {} // Initialize DiskManager with a default database file
        ~Executer() {}
        void execute_command(const StatementParser& stmt, DiskManager& disk); 
        void execute_insert(const StatementParser& stmt, DiskManager& disk); 
        void execute_select(const StatementParser& stmt, DiskManager& disk); 
        void execute_delete(const StatementParser& stmt, DiskManager& disk);
    private:
        StatementParser parser_; 
        //DiskManager disk_; 
        char read_buffer_[PAGE_SIZE] = {0};
        char write_buffer_[PAGE_SIZE] = {0};
}; 