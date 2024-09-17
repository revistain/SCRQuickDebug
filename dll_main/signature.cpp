#include "eud.h"
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

HANDLE getProcessHandle() { return hProcess; }
uint32_t getSignatureAddr() { return signature_address; }
uint32_t getBaseAddr() { return base_address; }
uint32_t getPacketAddr() { return packet_address; }
uint32_t getFuncTableAddr() { return func_table_address; }
uint32_t getVarTableAddr() { return var_table_address; }
uint32_t getVarDataAddr() { return var_data_address; }

// Function to convert a string to a vector of bytes
std::vector<uint8_t> StringToByteVector(const std::string& str) {
    return std::vector<uint8_t>(str.begin(), str.end());
}

// Function to open the process with the required access rights
bool OpenTargetProcess(DWORD processID) {
    hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, processID);
    if (hProcess == NULL) {
        std::cerr << "Failed to open process. Error code: " << GetLastError() << std::endl;
        return false;
    }
    return true;
}
#include <algorithm>

// 메모리 페이지 단위로 패턴을 찾는 함수
uint32_t FindPatternInMemory(HANDLE hProcess, const std::vector<uint8_t>& pattern) {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    MEMORY_BASIC_INFORMATION mbi;
    void* currentAddress = sysInfo.lpMinimumApplicationAddress;

    while (currentAddress < sysInfo.lpMaximumApplicationAddress) {
        // 메모리 페이지 정보 가져오기
        if (VirtualQueryEx(hProcess, currentAddress, &mbi, sizeof(mbi))) {
            // 메모리 페이지가 커밋 상태인지 확인 (읽기 가능한 페이지)
            if (mbi.State == MEM_COMMIT && (mbi.Protect & (PAGE_READWRITE | PAGE_READONLY | PAGE_EXECUTE_READ))) {
                std::vector<uint8_t> buffer(mbi.RegionSize);

                // 프로세스 메모리 읽기
                SIZE_T bytesRead;
                if (ReadProcessMemory(hProcess, mbi.BaseAddress, buffer.data(), mbi.RegionSize, &bytesRead)) {
                    // 검색할 메모리 범위에서 패턴 검색
                    auto it = std::search(buffer.begin(), buffer.begin() + bytesRead, pattern.begin(), pattern.end());
                    if (it != buffer.end()) {
                        // 패턴을 찾은 경우, 해당 메모리 주소를 반환
                        size_t offset = std::distance(buffer.begin(), it);
                        return reinterpret_cast<uint32_t>(mbi.BaseAddress) + offset;
                    }
                }
            }
            // 다음 페이지로 이동
            currentAddress = static_cast<uint8_t*>(mbi.BaseAddress) + mbi.RegionSize;
        }
        else {
            break;
        }
    }

    return 0;
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
        std::cerr << "Failed to create snapshot. Error code: " << GetLastError() << std::endl;
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
        std::cerr << "Process not found." << std::endl;
        return 0;
    }

    // Open the process
    if (!OpenTargetProcess(processID)) {
        return 0;
    }

    // Define the signature to search for
    std::string signature_str = "GongjknOSDIfnwlnlSNDKlnfkopqfnkLDNSFpwIn";
    std::vector<uint8_t> signature = StringToByteVector(signature_str);
    MEMORY_BASIC_INFORMATION mbi;
    //LPVOID address = reinterpret_cast<LPVOID>(0xFFFFFFFF); // Start from the top of the memory space
    uint32_t found_address = FindPatternInMemory(hProcess, signature);
    std::cout << "found: 0x" << std::hex << found_address << "\n";
    CloseHandle(hProcess);
    return 0;
}


void init_signature() {
    if (find_signature_address() != 0) {
        base_address = signature_address - unEPD(dwread(signature_address+40));
        packet_address = unEPD(dwread(signature_address + 44));
        func_table_address = dwread(signature_address + 48);
        var_table_address = dwread(signature_address + 52);
        var_data_address = dwread(signature_address + 56);
        std::cout << "base_address: 0x" << std::hex << packet_address << std::endl;
    }
    else {
        std::cerr << "Failed to find signature address." << std::endl;
    }
}

void end_signature() {
    CloseHandle(hProcess);
}