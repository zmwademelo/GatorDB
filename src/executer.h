#pragma once
#include "catalog/catalog.h" 
#include "storage/pager.h"
#include "common/statement.h"
#include "storage/storage_utils.h"

#include <iostream>
#include <string>

namespace Executer {
    void ExecuteCommand(const Statement& stmt, Pager& pager, Catalog& catalog); 

    void ExecuteCreate(const Statement& stmt, Catalog& catalog);
    void ExecuteDrop(const Statement& stmt, Catalog& catalog);
    void ExecuteInsert(const Statement& stmt, Pager& pager, const Catalog& catalog); 
    void ExecuteSelect(const Statement& stmt, const Pager& pager, const Catalog& catalog); 
    void ExecuteDelete(const Statement& stmt, Pager& pager, const Catalog& catalog);
    void ExecutePeek(const Statement& stmt, const Pager& pager); //For debug
}