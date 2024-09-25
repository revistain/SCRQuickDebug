#include "pattern.h"
#include <iostream>
#include <vector>
#include <thread>
#include <windows.h>
#include <mutex>
#include "../console/console.hpp"

// Mutex for safe access to foundAddresses
std::mutex foundMutex;
std::vector<LPCVOID> foundAddresses; // 저장할 메모리 주소
uint32_t getFoundAddr() {
    if (foundAddresses.size() > 1) throw std::string("multiple signature Found!!\n");
    return reinterpret_cast<uint32_t>(foundAddresses[0]);
}

const int ALPHABET_SIZE = 256;
std::vector<int> badCharTable(ALPHABET_SIZE, -1);
// Boyer-Moore의 bad character 테이블 생성
void buildBadCharTable(const std::vector<uint8_t>& needle) {
    for (int i = 0; i < needle.size( ); i++) {
        badCharTable[needle[i]] = i;
    }
}

// Boyer-Moore 알고리즘
int boyerMooreSearch(const std::vector<uint8_t>& haystack, std::vector<uint8_t>& needle, int start, int end) {
    int m = needle.size( );
    int n = haystack.size();
    if (m == 0 || n == 0 || m > n) return -1;

    std::vector<uint8_t> test;
    int shift = start;
    while (shift <= (end - m)) {
        int j = m - 1;
        // 패턴 매칭
        while (j >= 0 && (needle[j] == haystack[shift + j])) {
            j--;
        }

        if (j < 0) {
            return shift; // 패턴을 찾았을 때의 시작 위치
        } else {
            shift += max(1, j - badCharTable[haystack[shift + j]]);
        }
    }
    return -1; // 패턴을 찾지 못한 경우
}

// 메모리에서 프로세스 읽기
std::vector<uint8_t> readProcessMemory(HANDLE hProcess, LPCVOID baseAddress, SIZE_T size) {
    std::vector<uint8_t> buffer(size);
    SIZE_T bytesRead;

    if (ReadProcessMemory(hProcess, baseAddress, buffer.data(), size, &bytesRead)) {
        return buffer;
    }
    else {
        buffer.clear();
        return buffer;
    }
}

uint32_t readDwordProcessMemory(HANDLE hProcess, LPCVOID baseAddress) {
    uint32_t buffer = 0;
    SIZE_T bytesRead;

    if (ReadProcessMemory(hProcess, baseAddress, &buffer, 4, &bytesRead)) {
        return buffer;
    } else {
        return buffer;
    }
}

bool CheckStringInMemory(HANDLE hProcess, LPCVOID foundAddress) {
    char buffer[5] = {0};
    SIZE_T bytesRead;

    if (ReadProcessMemory(hProcess, (LPCVOID)((uintptr_t)foundAddress + 36), buffer, sizeof(buffer) - 1, &bytesRead) &&
        bytesRead == sizeof(buffer) - 1) {

        if (strcmp(buffer, "EDAC") == 0) {
            LOG("read: %s\n", buffer);
            return true;
        }
    }

    return false;
}

extern "C" __declspec(dllexport) void GetDllBaseAddress() {
    HMODULE hModule = GetModuleHandle(TEXT("eudplib_debug.dll"));
    if (hModule) {
        std::cout << "DLL Base Address: " << hModule << std::endl;
    } else {
        std::cerr << "Failed to get DLL handle." << std::endl;
    }
}


void searchMemory(HANDLE hProcess, std::vector<uint8_t>& signature, int numThreads) {
    LOG("needle address: 0x%08p\n", &signature);
    HMODULE hModule = GetModuleHandle(TEXT("eudplib_debug.dll"));
    if (!hModule) {
        throw std::string("Failed to get DLL handle\n");
    }
    LPCVOID dllBaseAddress = hModule;
    MODULEINFO modInfo;
    GetModuleInformation(GetCurrentProcess( ), hModule, &modInfo, sizeof(modInfo));
    SIZE_T dllSize = modInfo.SizeOfImage;

    foundAddresses.clear();
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    LPCVOID address = sysInfo.lpMinimumApplicationAddress;
    LPCVOID maxAddress = reinterpret_cast<LPCVOID>(0xFFFFFFFF);
    MEMORY_BASIC_INFORMATION mbi;
    std::cout << std::hex << "start address: 0x" << address << "  end: 0x" << maxAddress << "\n";
    
    // std::vector<std::thread> threads; // Uncomment if you decide to use threading
    buildBadCharTable(signature);
    while (address < maxAddress) {
        if (VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi)) == sizeof(mbi)) {
            if (mbi.State == MEM_COMMIT &&
                (mbi.Protect == PAGE_READWRITE ||
                 mbi.Protect == PAGE_READONLY ||
                 mbi.Protect == PAGE_EXECUTE_READ)) {

                if (address >= dllBaseAddress && address < (LPCVOID)((uintptr_t)dllBaseAddress + dllSize)) {
                    LOG("Skipping DLL memory: 0x%08X ~ 0x%08X\n",
                        mbi.BaseAddress,
                        (LPCVOID)((uintptr_t)mbi.BaseAddress + mbi.RegionSize));
                    address = (LPCVOID)((uintptr_t)mbi.BaseAddress + mbi.RegionSize);
                    continue;
                }

                SIZE_T regionSize = mbi.RegionSize;
                std::vector<uint8_t> buffer = readProcessMemory(hProcess, mbi.BaseAddress, regionSize);

                if (!buffer.empty( )) {
                    int result = boyerMooreSearch(buffer, signature, 0, buffer.size( ));
                    if (result != -1) {
                        LPCVOID foundAddress = (LPCVOID)((uintptr_t)mbi.BaseAddress + result);
                        
                        if (CheckStringInMemory(hProcess, foundAddress)) {
                            LOG("Pattern found at: 0x%08X ~ 0x%08X\n",
                                mbi.BaseAddress,
                                (LPCVOID)((uintptr_t)mbi.BaseAddress + mbi.RegionSize));
                            foundAddresses.push_back(foundAddress);
                        } else {
                            LOG("Pattern found but skipped at: 0x%08X ~ 0x%08X, 0x%08X\n",
                                mbi.BaseAddress,
                                (LPCVOID)((uintptr_t)mbi.BaseAddress + mbi.RegionSize), foundAddress);
                        }
                    }

                    /*
                    threads.emplace_back([&, regionSize, buffer, mbi]() {
                        int result = boyerMooreSearch(buffer, signature, 0, buffer.size());
                        if (result != -1) {
                            LPCVOID foundAddress = (LPCVOID)((uintptr_t)mbi.BaseAddress + result);
                            std::lock_guard<std::mutex> lock(foundMutex);
                            foundAddresses.push_back(foundAddress);
                        }
                    });
                    */
                }
            }
            address = (LPCVOID)((uintptr_t)mbi.BaseAddress + mbi.RegionSize);
        } else {
            break;
        }
    }

    /*
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    */

    if (!foundAddresses.empty()) {
        for (const auto& addr : foundAddresses) {
            std::cout << "Pattern found at address: 0x" << std::hex << addr << std::endl;
        }
    }
    else {
        throw std::string("Pattern not found\n");
    }
}