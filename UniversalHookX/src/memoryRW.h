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

bool OpenTargetProcess( );
bool CloseTargetProcess( );
uint32_t getEXEAddr( );
uint32_t findMRGNAddr(std::string map_path);

namespace Internal {
    uint32_t dwread(uint32_t address);
    uint16_t wread(uint32_t address);
    uint8_t bread(uint32_t address);
    bool dwwrite(uint32_t address, uint32_t value);
    bool wwrite(uint32_t address, uint16_t value);
    bool bwrite(uint32_t address, uint8_t value);
    bool vecwrite(uint32_t address, std::vector<uint8_t> value);
    uint32_t choosePathAddr(const std::vector<uint32_t> foundaddresses);
    std::vector<uint32_t> searchMemory(HANDLE hProcess, std::vector<uint8_t>& signature);
} // namespace Internal

#endif