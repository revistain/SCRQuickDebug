#ifndef PATTERN_H
#define PATTERN_H 
#include <windows.h>
#include <vector>
#include <iostream>

void searchMemory(HANDLE hProcess, const std::vector<uint8_t>& signature, int numThreads);
#endif