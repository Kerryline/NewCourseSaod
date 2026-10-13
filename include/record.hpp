#pragma once
#include <cstddef>

#pragma pack(push, 1)
struct Record {
    char fio[32];
    char street[18];
    short house;
    short apartment;
    char date[10];
};
#pragma pack(pop)

static_assert(sizeof(Record) == 64, "Record size must be 64");