// Forg: Includes should be ordered. Check the Gemini conversation on how to use the extension to auto do it.

#include <iostream>
#include <cstring>
#include <memory>

#include "storage/disk_manager.h"
#include "storage/pager.h"
#include "storage/catalog.h"
#include "common/statement.h"
#include "executer.h"

int main() {
    // Frog: Give a comment to literal argumnent, i.e. /* db_file= */"gator_db.db".
    //auto disk = std::make_unique<DiskManager>(/* db_file = */"gator_db.db"); 
    // Frog: This means pager owns disk manager, which usually means pager is the only object that talks to disk manager. If that's intended, it is fine.
    // Frog: If pager indeed owns disk manager, you can consider improving code clarity, by letting Pager factory method takes the db_file name, and create the
    // disk manager within Pager's factory method. This means DiskManager is essentially a private helper class of Pager. 
    auto pager = std::make_unique<Pager>("gator_db.db"); 
    // Frog: This means that catalog owns Pager, which does not sound correct. IIUC, Executor also talks to pager (for non-catalog related operations, like select).
    // I'd suggest letting catalog takes a raw pointer to Pager, and add a comment: "Pager must outlive *this".
    auto catalog = std::make_unique<Catalog>(pager.get()); 

    std::cout << "Welcome to gator DB! Type 'help' for commands. Use EXIT/QUIT to exit." << std::endl; 
            while (true){
                std::cout << "gator > ";
                std::string input;
                std::getline(std::cin, input); 
                // Frog: StatementParser should return a Statement, not a StatementParser.
                std::unique_ptr<StatementParser> stmt = StatementParser::parse_statement(input); 

                // Frog: Again, I am strongly against using nullptr to represent error. Should use absl::StatusOr<std::unique_ptr>>.
                if (stmt) {
                    // Frog: Consider making executor not a class, but just a free function, since it is so short-living?
                    auto executer = Executer(catalog->get_pager(), *catalog);
                    executer.execute_command(*stmt);
                }
                // Frog: How about "else"? Shouldn't the else branch handle the error, like printing the problem or making the program exit?
            }

    return 0;
}