#ifndef CUNIT_H
#define CUNIT_H
#include <vector>
#include <iostream>
#include <unordered_map>
#include "unittable.h"
#include "signature.h"

class CUnits {
public:
    bool isDisplayingCUnit[1700];
    std::unordered_map<uint32_t, bool> isDisplayingCSprite;
    std::vector<CUnit> cunits;
    std::vector<CSprite> csprites;
    uint32_t start_address;

    CUnits(uint32_t _start_address);
    void update_cunit(uint32_t idx);
    void update( );
};

#endif