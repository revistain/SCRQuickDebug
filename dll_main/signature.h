#ifndef SIGNATURE_H
#define SIGNATURE_H
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>
#include <string>

void init_signature();
void end_signature();

HANDLE getProcessHandle();
uint32_t getSignatureAddr();
uint32_t getBaseAddr();
uint32_t getPacketAddr();
uint32_t getFuncTableAddr();
uint32_t getVarTableAddr();
uint32_t getVarDataAddr();

std::string strread(uint32_t address, size_t size);
uint32_t dwread(uint32_t address);
#endif
