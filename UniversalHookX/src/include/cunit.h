#ifndef CUNIT_H
#define CUNIT_H
#include <vector>
#include <iostream>
#include "unittable.h"
#include "signature.h"

class CUnits {
public:
    bool isDisplaying[1700];
    std::vector<CUnit> cunits;
    std::vector<CUnit> cunits_buffer;
    std::vector<uint32_t> display_stack;
    uint32_t start_address;
    CUnits(uint32_t _start_address);

    void update_cunit(uint32_t idx);
    void update( );
};

#endif