#pragma once
#include <cstdint>
#include "common/config.h"

/*
You create a single "address" object that can be passed between the Executer, TablePage, and any future Index classes.
*/
struct RID {
    page_id_t page_id; // The page number where the record is stored
    uint16_t slot_num; // The slot number within the page where the record is stored

    RID() : page_id(0), slot_num(0) {}
    RID(page_id_t pg_id, uint16_t s_num) : page_id(pg_id), slot_num(s_num) {}

    bool operator==(const RID &other) const {
        return page_id == other.page_id && slot_num == other.slot_num;
    }
};