#ifndef PATTERN_H
#define PATTERN_H 
#include <windows.h>
#include <vector>
#include <iostream>
#include <psapi.h>

#pragma comment(lib, "Psapi.lib")
void searchMemory(HANDLE hProcess, std::vector<uint8_t>& signature, int numThreads);
uint32_t getFoundAddr();
#endif