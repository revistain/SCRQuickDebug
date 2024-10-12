#ifndef SIGNATURE_H
#define SIGNATURE_H
#include "pattern.h"
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

void init_signature();
void end_signature();
bool OpenTargetProcess();
std::vector<uint8_t> StringToByteVector(std::string& str);
uint32_t GetModuleBaseAddress(const wchar_t* modName);

HANDLE getProcessHandle();
uint32_t getEXEAddr();
void setEXEAddr(uint32_t paddr);
uint32_t getUnittableAddr( );
void setUnittableAddrOpenGL(uint32_t paddr);
void setUnittableAddr(uint32_t paddr);
uint32_t getSignatureAddr( );
uint32_t getBaseAddr( );
uint32_t getPacketAddr( );
uint32_t getStringAddr( );
uint32_t getVarAddr( );
uint32_t getGVarAddr( );
uint32_t getArrAddr( );
uint32_t getGArrAddr( );
uint32_t getMRGNDataAddr( );
uint32_t getScreenDataAddr( );
uint32_t getWireFrameDataAddr( );


std::string strread(uint32_t address, size_t size);
uint32_t dwread(uint32_t address);
uint16_t wread(uint32_t address);
uint8_t bread(uint32_t address);
bool ReadMemory(LPCVOID address, SIZE_T length, BYTE* buffer);
bool dwwrite(uint32_t address, uint32_t value);
bool wwrite(uint32_t address, uint16_t value);
bool bwrite(uint32_t address, uint8_t value);
bool vecwrite(uint32_t address, std::vector<uint8_t> value);

#endif
