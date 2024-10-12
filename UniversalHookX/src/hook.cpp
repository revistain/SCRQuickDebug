#include "hook.h"
#include <iostream>
void* FindFuncPattern(unsigned char* base, size_t size, const char* pattern, const char* mask) {
    for (size_t i = 0; i < size; ++i) {
        bool found = true;
        for (size_t j = 0; pattern[j] != '\0'; ++j) {
            if (mask[j] == 'x' && base[i + j] != pattern[j]) {
                found = false;
                break;
            }
        }
        if (found) {
            std::cout << "found: " << std::hex << found << "\n";
            return &base[i];
        }
    }
    return nullptr;
}
    
void HookFunction(void* targetFunc, void* myFunc) {
    DWORD oldProtect;
    VirtualProtect(targetFunc, 5, PAGE_EXECUTE_READWRITE, &oldProtect); // Change memory protection to write

    // Calculate relative offset
    int32_t relativeAddress = (int32_t)myFunc - (int32_t)targetFunc - 5;

    // Insert a JMP instruction (opcode 0xE9) to jump to myFunc
    *(uint8_t*)targetFunc = 0xE9;
    *(int32_t*)((uint8_t*)targetFunc + 1) = relativeAddress;

    VirtualProtect(targetFunc, 5, oldProtect, &oldProtect); // Restore memory protection
}
