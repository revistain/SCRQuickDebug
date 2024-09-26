#ifndef MEMORYRW_H
#define MEMORYRW_H
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <iostream>
#include <cstdint>
#include <vector>
#include "pattern.h"
#include "signature.h"
#include "LocationDraw.h"

uint32_t getEXEAddr( );
uint32_t findMRGNAddr();
uint32_t findUnitableAddr( );
uint32_t getElapsedTime(uint32_t mrgn_addr);

namespace Internal {
    std::vector<uint8_t> vecread(uint32_t address, size_t size);
    uint32_t dwread(uint32_t address);
    uint16_t wread(uint32_t address);
    uint8_t bread(uint32_t address);
    bool vecwrite(uint32_t address, std::vector<uint8_t> value);
    bool dwwrite(uint32_t address, uint32_t value);
    bool wwrite(uint32_t address, uint16_t value);
    bool bwrite(uint32_t address, uint8_t value);
    uint32_t choosePathAddr(const std::vector<uint32_t> foundaddresses);
    std::vector<uint32_t> searchMemory(HANDLE hProcess, std::vector<uint8_t>& signature);
    std::vector<uint32_t> searchAllocationMemory(HANDLE hProcess);
} // namespace Internal

#endif