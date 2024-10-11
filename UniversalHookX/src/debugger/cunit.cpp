#include "cunit.h"

CUnits::CUnits(uint32_t _start_address) : start_address(_start_address) {
    std::cout << "********** start addr : 0x" << std::hex << _start_address + 8 << "\n";
    if (_start_address == 0x60000) {
        start_address = 0;
        return;
    }
    cunits.resize(1700);
}
void CUnits::update_cunit(uint32_t idx) {
    ReadMemory((LPCVOID)(start_address + 8 + idx * 0x150), 0x150, (BYTE*)&cunits[idx]);
}
void CUnits::update( ) {
    // 1. only need ones
    /*
    for (size_t idx = 0; idx < 1700; idx++) {
        if (needUpdate[idx]) update_cunit(idx);
    }
    */

    // 2. or just update all, cpp is fast
    ReadMemory((LPCVOID)(start_address + 8), 0x150*1700, (BYTE*)&cunits[0]);
}