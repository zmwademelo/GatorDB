#include <iostream>
#include <cstring>
#include <memory>
#include "storage/disk_manager.h"
#include "common/statement.h"
#include "src/executer.h"

int main() {
    DiskManager disk("test_db.db");

    /*   
    char write_buffer[PAGE_SIZE];//array of characters to hold data to write
    std::memset(write_buffer, 0, PAGE_SIZE); // Initialize buffer with zeros
    
    // Simulate a record
    std::string test_data = "Hello Database!";
    std::memcpy(write_buffer, test_data.c_str(), test_data.length());//write_buffer is actually a pointer to the first element of the array

    // Write to Page 0
    disk.WritePage(0, write_buffer);  //write_buffer has to point to a memory chunk of PAGE_SIZE bytes
    std::cout << "Wrote to disk." << std::endl;

    //cannot write two pages at pn
    char second_buffer[PAGE_SIZE];
    std::memset(second_buffer, 0, PAGE_SIZE); // Clear second buffer


    std::memcpy(second_buffer, "Team Vitality! ", 15); //Bad practice. If length does not match, it may lead to buffer overflow or other unexpected behaviors.

    disk.WritePage(1, second_buffer); // Write empty page to Page 1
    std::cout << "Wrote Vitality to page2 ." << std::endl;

    // Read back into a fresh buffer
    char read_buffer[PAGE_SIZE];
    disk.ReadPage(0, read_buffer);

    std::cout << "Read from disk: " << read_buffer << std::endl; 

    while (true){
        std::cout << "Gator > "<< "(Press q to quit: )" << std::endl; 
        std::string input;
        std::getline(std::cin, input);

    }
    */

    std::cout << "Welcome to gator DB! Type 'help' for commands. Use EXIT/QUIT to exit." << std::endl; 
            while (true){
                std::cout << "gator > ";
                std::string input;
                std::getline(std::cin, input); 
                std::unique_ptr<StatementParser> stmt(new StatementParser());
                stmt->parse_statement(input);
                //Executer executer = Executer(*stmt);
                std::unique_ptr<Executer> executer(new Executer(*stmt));
                executer->execute_command(*stmt, disk);
                //delete stmt;
            }

    return 0;
}