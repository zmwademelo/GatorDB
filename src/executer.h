#pragma once
#include "catalog/catalog.h" 
#include "storage/pager.h"
#include "common/statement.h"
#include "storage/storage_utils.h"

#include <iostream>
#include <string>

namespace Executer {
    void execute_command(const Statement& stmt, Pager& pager, Catalog& catalog); 

    void execute_create(const Statement& stmt, Catalog& catalog);
    void execute_drop(const Statement& stmt, Catalog& catalog);
    void execute_insert(const Statement& stmt, Pager& pager, const Catalog& catalog); 
    void execute_select(const Statement& stmt, const Pager& pager, const Catalog& catalog); 
    void execute_delete(const Statement& stmt, Pager& pager, const Catalog& catalog);
    void execute_peek(const Statement& stmt, const Pager& pager); //For debug
}