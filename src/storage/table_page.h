#pragma once
#include "disk_manager.h"
#include "pager.h"
#include "common/rid.h"
#include <string>
#include <iostream>

/*
The Physical Memory Layout
Think of your TablePage as a divided container. The directory starts at the "top" (just after the metadata), and the actual data strings start at the very "bottom" and work their way up.

Byte 0 to sizeof(Header): The TablePageHeader.

Next section: The Slot array (the Slot Directory).

The Gap: Free space for growth.

End of Page: The actual record data.
*/

class TablePage {
    friend class Pager; //Pager needs to access the private members of TablePage to manage page-level metadata and operations effectively.
    public:
        struct PageHeader {
            uint32_t magic_number; 
            uint16_t lower_bound; 
            uint16_t upper_bound;
            uint16_t slot_count; 
            uint16_t free_space;
        };
        struct Slot{
            uint16_t offset; 
            int16_t length; //negative length means the slot is deleted
        }; 
        explicit TablePage(char* data) : data_(data) {
        }; 

        void initialize_empty_page();

        uint16_t insert_record(const char* record_data);
        std::string get_record(const uint16_t slot_num);
        bool delete_record(const uint16_t slot_num);

        uint16_t get_record_count() {
            return get_page_header()->slot_count;   
        }

        uint16_t get_slot_count() {
            return get_page_header()->slot_count; 
        
        }
        Slot* get_slot(uint16_t slot_num) {
            if (slot_num >= get_slot_count()) {
                return nullptr; 
            }
            return &get_slot_directory()[slot_num]; 
        }
    private: 
        PageHeader* get_page_header(){
            return reinterpret_cast<PageHeader*>(data_); //reinterpret_cast is used to convert the raw byte data into a structured format that we can work with. 
            //It allows us to treat the first few bytes of the page as a PageHeader, giving us easy access to the metadata about the page.
        }
        Slot* get_slot_directory(){
            return reinterpret_cast<Slot*>(data_ + sizeof(PageHeader)); //This points to the start of the slot directory, which is located immediately after the page header in the page's byte layout. 
            //By using reinterpret_cast, we can treat this portion of the page as an array of Slot structures, allowing us to easily manage and access the slot information for each record stored in the page. 
            //Because every Slot is the same size (4 bytes in this example), the compiler can calculate exactly where slots[i] is by doing: Start_Address + (i * sizeof(Slot)). 
        }
        char* data_; // The actual 4KB buffer
        //We can get the slot_count by 
};
