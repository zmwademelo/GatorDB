// Forg: Includes should be ordered. Check the Gemini conversation on how to use
// the extension to auto do it.

#include <cstring>
#include <iostream>
#include <memory>

#include "common/statement.h"
#include "executer.h"
#include "catalog/catalog.h"
#include "storage/disk_manager.h"
#include "storage/pager.h"

int main() {

  auto pager = Pager::create("gator_db.db");
  // Pager must outlive *this
  auto catalog = std::make_unique<Catalog>(pager.get());

  std::cout
      << "Welcome to gator DB! Type 'help' for commands. Use EXIT/QUIT to exit."
      << std::endl;
  while (true) {
    std::cout << "gator > ";
    std::string input;
    std::getline(std::cin, input);
    std::unique_ptr<Statement> stmt =
        Statement::parse_statement(input);

    // Frog: Again, I am strongly against using nullptr to represent error.
    // Should use absl::StatusOr<std::unique_ptr>>.
    if (stmt) {
      Executer::execute_command(*stmt, *pager, *catalog);
    } else {
        std::cout << "Error getting command. " << std::endl; 
        continue; 
    }
  }

  return 0;
}