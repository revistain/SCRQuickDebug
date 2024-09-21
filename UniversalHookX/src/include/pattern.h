#ifndef PATTERN_H
#define PATTERN_H 
#include <windows.h>
#include <vector>
#include <iostream>
#include <psapi.h>

#pragma comment(lib, "Psapi.lib")
void buildBadCharTable(const std::vector<uint8_t>& needle);
std::vector<uint8_t> readProcessMemory(HANDLE hProcess, LPCVOID baseAddress, SIZE_T size);
int boyerMooreSearch(const std::vector<uint8_t>& haystack, std::vector<uint8_t>& needle, int start, int end);
void searchMemory(HANDLE hProcess, std::vector<uint8_t>& signature, int numThreads);
uint32_t readDwordProcessMemory(HANDLE hProcess, LPCVOID baseAddress);
uint32_t getFoundAddr();
#endif