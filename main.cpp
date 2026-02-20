#include <iostream>
#include <cstring>
#include <memory>

#include "storage/disk_manager.h"
#include "storage/pager.h"
#include "storage/catalog.h"
#include "common/statement.h"
#include "src/executer.h"

int main() {
    auto disk = std::make_unique<DiskManager>("gator_db.db"); 
    auto pager = std::make_unique<Pager>(disk.get()); 
    auto catalog = std::make_unique<Catalog>(pager.get()); 

    std::cout << "Welcome to gator DB! Type 'help' for commands. Use EXIT/QUIT to exit." << std::endl; 
            while (true){
                std::cout << "gator > ";
                std::string input;
                std::getline(std::cin, input); 
                std::unique_ptr<StatementParser> stmt(new StatementParser());
                if (stmt->parse_statement(input)) {
                    auto executer = std::make_unique<Executer>(*pager, *catalog);
                    executer->execute_command(*stmt);
                }
                //Executer executer = Executer(*stmt);
                
                //delete stmt;
            }

    return 0;
}