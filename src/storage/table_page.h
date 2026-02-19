#pragma once
#include "disk_manager.h"
#include "tuple.h"
#include "pager.h"
#include "common/rid.h"
#include <string>
#include <iostream>
#include <memory>

/*
class TablePage {
    
    public:
        
        explicit TablePage(char* data) : data_(data) {
        }; 

        void initialize_empty_page();

        uint16_t insert_record(Tuple tuple);
        //uint16_t insert_record(const char* record_data, uint16_t record_length); //Accept length to prevent only the first char is passed
        std::string get_record(const uint16_t slot_num);
        bool delete_record(const uint16_t slot_num);

        uint16_t get_slot_count() {
            return get_page_header()->slot_count; 
        }

        

        page_id_t get_next_page_id();
        page_id_t set_next_page_id(); 

        Slot* get_slot(uint16_t slot_num) {
            if (slot_num >= get_slot_count()) {
                return nullptr;
            }
            return &get_slot_directory()[slot_num]; 
       }

       PageHeader* get_page_header() {
            //return reinterpret_cast<PageHeader*>(data_); //reinterpret_cast is used to convert the raw byte data into a structured format that we can work with. 
            return reinterpret_cast<PageHeader*>(data_);
            //It allows us to treat the first few bytes of the page as a PageHeader, giving us easy access to the metadata about the page.
        }
    private: 
        //PageHeader* get_page_header(){
        
    
        //Slot* get_slot_directory(){
        Slot* get_slot_directory() {
            return reinterpret_cast<Slot*>(data_ + sizeof(PageHeader));
            //return reinterpret_cast<Slot*>(data_ + sizeof(PageHeader)); 
            //This points to the start of the slot directory, which is located immediately after the page header in the page's byte layout. 
            //By using reinterpret_cast, we can treat this portion of the page as an array of Slot structures, allowing us to easily manage and access the slot information for each record stored in the page. 
            //Because every Slot is the same size (4 bytes in this example), the compiler can calculate exactly where slots[i] is by doing: Start_Address + (i * sizeof(Slot)). 
        }
        char* data_; // The actual 4KB buffer
        //std::unique_ptr<char[]> data;  
        //We can get the slot_count by 
};
*/