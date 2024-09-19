#include "eud.h"
#include "signature.h"
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>
#include <string>
#include "../console/console.hpp"

static HANDLE hProcess;  // Global handle to the process
uint32_t signature_address;
uint32_t base_address;
uint32_t packet_address;
uint32_t func_table_address;
uint32_t var_table_address;
uint32_t var_data_address;
uint32_t mrgn_table_address;
uint32_t mrgn_data_address;
uint32_t screen_data_address;
uint32_t map_path_address;

uint32_t exeAddr = 0;
uint32_t processID = 0;

HANDLE getProcessHandle( ) { return hProcess; }
uint32_t getEXEAddr( ) { return exeAddr; }
uint32_t getSignatureAddr() { return signature_address; }
uint32_t getBaseAddr() { return base_address; }
uint32_t getPacketAddr() { return packet_address; }
uint32_t getFuncTableAddr() { return func_table_address; }
uint32_t getVarTableAddr() { return var_table_address; }
uint32_t getVarDataAddr() { return var_data_address; }
uint32_t getMRGNTableAddr() { return mrgn_table_address; }
uint32_t getMRGNDataAddr() { return mrgn_data_address; }
uint32_t getScreenDataAddr() { return screen_data_address; }
uint32_t getMapPathAddr() { return map_path_address; }

std::vector<uint8_t> StringToByteVector(std::string& str) {
    std::vector<uint8_t> ret = std::vector<uint8_t>(str.begin( ), str.end( ));
    str.resize(40, '0');
    return ret;
}

void GetModuleBaseAddress(const wchar_t* modName) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);
    if (hSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry)) {
            do {
                if (!_wcsicmp(modEntry.szModule, modName)) {
                    exeAddr = (uintptr_t)modEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    }
    CloseHandle(hSnap);
}

bool OpenTargetProcess() {
    if (processID) return true;
    std::wstring processName = L"StarCraft.exe";

    // Get process ID
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        throw "Failed to create snapshot!";
        return 0;
    }

    PROCESSENTRY32 pe = {0};
    pe.dwSize = sizeof(pe);

    if (Process32First(hSnapshot, &pe)) {
        do {
            if (!_wcsicmp(pe.szExeFile, processName.c_str( ))) {
                processID = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);

    if (processID == 0) {
        return false;
    }

    hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, processID);
    if (hProcess == NULL) {
        std::cerr << "Failed to open process. Error code: " << GetLastError() << std::endl;
        return false;
    }
    return true;
}

bool ReadMemory(LPCVOID address, SIZE_T length, BYTE* buffer) {
    SIZE_T bytesRead;
    return ReadProcessMemory(hProcess, address, buffer, length, &bytesRead) && bytesRead == length;
}

bool WriteMemory(LPVOID address, SIZE_T length, BYTE* buffer) {
    SIZE_T bytesWrite;
    return WriteProcessMemory(hProcess, address, buffer, length, &bytesWrite) && bytesWrite == length;
}

std::string strread(uint32_t address, size_t size) {
    std::vector<char> buf(size);
    if (ReadMemory(reinterpret_cast<LPCVOID>(address), size, reinterpret_cast<BYTE*>(buf.data()))) {
        return std::string(buf.data(), size);
    }
    else {
        std::cout << "Error: Failed to read memory from address " << address << ".\n";
        return std::string();
    }
}

uint32_t dwread(uint32_t address) {
    uint32_t value = 0;
    if (ReadMemory(reinterpret_cast<LPCVOID>(address), sizeof(value), reinterpret_cast<BYTE*>(&value))) {
        return value;
    }
    else {
        std::cout << "Error: Failed to read 4bytes from memory.\n";
        return 0;
    }
}

uint16_t wread(uint32_t address) {
    uint16_t value = 0;
    if (ReadMemory(reinterpret_cast<LPCVOID>(address), sizeof(value), reinterpret_cast<BYTE*>(&value))) {
        return value;
    }
    else {
        std::cout << "Error: Failed to read 2bytes from memory.\n";
        return 0;
    }
}

uint8_t bread(uint32_t address) {
    uint8_t value = 0;
    if (ReadMemory(reinterpret_cast<LPCVOID>(address), sizeof(value), reinterpret_cast<BYTE*>(&value))) {
        return value;
    }
    else {
        std::cout << "Error: Failed to read byte from memory.\n";
        return 0;
    }
}


bool dwwrite(uint32_t address, uint32_t value) {
    if(WriteMemory(reinterpret_cast<LPVOID>(address), 4,  reinterpret_cast<BYTE*>(&value))) {
        return true;
    }
    else {
        std::cout << "Error: Failed to write 4bytes to memory.\n";
        return false;
    }
}

bool wwrite(uint32_t address, uint16_t value) {
    if (WriteMemory(reinterpret_cast<LPVOID>(address), 2, reinterpret_cast<BYTE*>(&value))) {
        return true;
    }
    else {
        std::cout << "Error: Failed to write 2bytes to memory.\n";
        return false;
    }
}

bool bwrite(uint32_t address, uint8_t value) {
    if (WriteMemory(reinterpret_cast<LPVOID>(address), 1, reinterpret_cast<BYTE*>(&value))) {
        return true;
    }
    else {
        std::cout << "Error: Failed to write byte to memory.\n";
        return false;
    }
}

bool vecwrite(uint32_t address, std::vector<uint8_t> value) {
    if (WriteMemory(reinterpret_cast<LPVOID>(address), value.size(), reinterpret_cast<BYTE*>(&value))) {
        return true;
    }
    else {
        std::cout << "Error: Failed to write vec to memory.\n";
        return false;
    }
}

// Function to find the signature address
uint32_t find_signature_address() {
    // Open the process
    if (!OpenTargetProcess()) {
        return 0;
    }

    std::string signature_str("GongjknOSDIfnwlnlSNDKlnfkopqfnkLDNSF");
    std::vector<uint8_t> signature = StringToByteVector(signature_str);
    LOG("Seaching start\n");
    searchMemory(hProcess, signature, 10);

    return getFoundAddr( );
}


void init_signature() {
    try {
        signature_address = find_signature_address();
        LOG("found addr: 0x%08X\n", signature_address);
        if (signature_address != 0) {
            base_address = signature_address - unEPD(dwread(signature_address + 40));
            packet_address = base_address + unEPD(dwread(signature_address + 44));
            func_table_address = base_address + dwread(signature_address + 48);
            var_table_address = base_address + dwread(signature_address + 52);
            var_data_address = base_address + dwread(signature_address + 56);
            mrgn_table_address = base_address + dwread(signature_address + 60);
            mrgn_data_address = base_address + dwread(signature_address + 64);
            screen_data_address = base_address + unEPD(dwread(signature_address + 68));
            map_path_address = base_address + unEPD(dwread(signature_address + 72));
        }
        else { std::cerr << "cannot find signature address\n"; }
    }
    catch (const char* e) {
        throw "Error finding signature";
    }
}

void end_signature() {
    CloseHandle(hProcess);
    processID = 0;
}