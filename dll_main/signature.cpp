#include "eud.h"
#include "signature.h"
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>
#include <string>

HANDLE hProcess;  // Global handle to the process
uint32_t signature_address;
uint32_t base_address;
uint32_t packet_address;
uint32_t func_table_address;
uint32_t var_table_address;
uint32_t var_data_address;
uint32_t mrgn_table_address;
uint32_t mrgn_data_address;

HANDLE getProcessHandle() { return hProcess; }
uint32_t getSignatureAddr() { return signature_address; }
uint32_t getBaseAddr() { return base_address; }
uint32_t getPacketAddr() { return packet_address; }
uint32_t getFuncTableAddr() { return func_table_address; }
uint32_t getVarTableAddr() { return var_table_address; }
uint32_t getVarDataAddr() { return var_data_address; }
uint32_t getMRGNTableAddr() { return mrgn_table_address; }
uint32_t getMRGNDataAddr() { return mrgn_data_address; }

// Function to convert a string to a vector of bytes
std::vector<uint8_t> StringToByteVector(const std::string& str) {
    return std::vector<uint8_t>(str.begin(), str.end());
}

// Function to open the process with the required access rights
bool OpenTargetProcess(DWORD processID) {
    hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, processID);
    if (hProcess == NULL) {
        std::cerr << "Failed to open process. Error code: " << GetLastError() << std::endl;
        return false;
    }
    return true;
}

// Function to read a block of memory from the process
bool ReadMemory(LPCVOID address, SIZE_T length, BYTE* buffer) {
    SIZE_T bytesRead;
    return ReadProcessMemory(hProcess, address, buffer, length, &bytesRead) && bytesRead == length;
}
std::string strread(uint32_t address, size_t size) {
    std::vector<char> buf(size);
    if (ReadMemory(reinterpret_cast<LPCVOID>(address), size, reinterpret_cast<BYTE*>(buf.data()))) {
        return std::string(buf.data(), size);
    }
    else {
        std::cerr << "Error: Failed to read memory from address " << address << ".\n";
        return std::string();
    }
}

// Function to read 4 bytes as uint32_t
uint32_t dwread(uint32_t address) {
    uint32_t value = 0;
    if (ReadMemory(reinterpret_cast<LPCVOID>(address), sizeof(value), reinterpret_cast<BYTE*>(&value))) {
        return value;
    }
    else {
        std::cerr << "Error: Failed to read 4 bytes from memory.\n";
        DWORD error = GetLastError();
        std::cout << "hProcess: 0x" << std::hex << hProcess << std::endl;
        std::cerr << "ReadProcessMemory failed with error: " << error << "\n";
        return 0; // Return 0 or some error value
    }
}

// Function to read 2 bytes as uint16_t
uint16_t wread(uint32_t address) {
    uint16_t value = 0;
    if (ReadMemory(reinterpret_cast<LPCVOID>(address), sizeof(value), reinterpret_cast<BYTE*>(&value))) {
        return value;
    }
    else {
        std::cerr << "Error: Failed to read 2 bytes from memory.\n";
        return 0; // Return 0 or some error value
    }
}

// Function to read 1 byte as uint8_t
uint8_t bread(uint32_t address) {
    uint8_t value = 0;
    if (ReadMemory(reinterpret_cast<LPCVOID>(address), sizeof(value), reinterpret_cast<BYTE*>(&value))) {
        return value;
    }
    else {
        std::cerr << "Error: Failed to read 1 byte from memory.\n";
        return 0; // Return 0 or some error value
    }
}

// Function to find the signature address
uint32_t find_signature_address() {
    std::wstring processName = L"StarCraft.exe";
    DWORD processID = 0;

    // Get process ID
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        throw "Failed to create snapshot!";
        return 0;
    }

    PROCESSENTRY32 pe = { 0 };
    pe.dwSize = sizeof(pe);

    if (Process32First(hSnapshot, &pe)) {
        do {
            if (!_wcsicmp(pe.szExeFile, processName.c_str())) {
                processID = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);

    if (processID == 0) {
        throw "Process not found!";
    }

    // Open the process
    if (!OpenTargetProcess(processID)) {
        throw "cannot open process!";
    }

    std::string signature_str = "GongjknOSDIfnwlnlSNDKlnfkopqfnkLDNSFpwIn";
    std::vector<uint8_t> signature = StringToByteVector(signature_str);
    searchMemory(hProcess, signature, 10);
    return getFoundAddr();
}


void init_signature() {
    try {
        signature_address = find_signature_address();
        if (signature_address != 0) {
            base_address = signature_address - unEPD(dwread(signature_address + 40));
            packet_address = base_address + unEPD(dwread(signature_address + 44));
            func_table_address = base_address + dwread(signature_address + 48);
            var_table_address = base_address + dwread(signature_address + 52);
            var_data_address = base_address + dwread(signature_address + 56);
            mrgn_table_address = base_address + dwread(signature_address + 60);
            mrgn_data_address = base_address + dwread(signature_address + 64);
            std::cout << "base_address: 0x" << std::hex << base_address << std::endl;
            std::cout << "packet_address: 0x" << std::hex << packet_address << std::endl;
            std::cout << "func_table_address: 0x" << std::hex << func_table_address << std::endl;
            std::cout << "var_table_address: 0x" << std::hex << var_table_address << std::endl;
            std::cout << "var_data_address: 0x" << std::hex << var_data_address << std::endl;

        }
        else { throw "found Signature is 0x00000000"; }
    }
    catch (const char* e) {
        CloseHandle(hProcess);
        throw "Error finding signature";
    }
}

void end_signature() {
    CloseHandle(hProcess);
}