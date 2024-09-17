#include "pattern.h"
#include <iostream>
#include <vector>
#include <thread>
#include <windows.h>
#include <mutex>

// Mutex for safe access to foundAddresses
std::mutex foundMutex;
std::vector<LPCVOID> foundAddresses; // 저장할 메모리 주소

// Boyer-Moore의 bad character 테이블 생성
std::vector<int> buildBadCharTable(const std::vector<uint8_t>& needle) {
    const int ALPHABET_SIZE = 256;
    std::vector<int> badCharTable(ALPHABET_SIZE, -1);

    for (int i = 0; i < needle.size(); i++) {
        badCharTable[needle[i]] = i;
    }

    return badCharTable;
}

// Boyer-Moore 알고리즘
int boyerMooreSearch(const std::vector<uint8_t>& haystack, const std::vector<uint8_t>& needle, int start, int end) {
    int m = needle.size();
    int n = haystack.size();

    if (m == 0 || n == 0 || m > n) return -1;

    std::vector<int> badCharTable = buildBadCharTable(needle);

    int shift = start;
    while (shift <= (end - m)) {
        int j = m - 1;

        while (j >= 0 && needle[j] == haystack[shift + j]) {
            j--;
        }

        if (j < 0) {
            return shift; // 패턴을 찾았을 때의 시작 위치
        }
        else {
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

// 메모리 페이지 단위로 검색하는 함수
void searchMemory(HANDLE hProcess, const std::vector<uint8_t>& signature, int numThreads) {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo); // 시스템 정보 가져오기

    LPCVOID address = sysInfo.lpMinimumApplicationAddress; // 검색할 메모리의 시작 주소
    LPCVOID maxAddress = sysInfo.lpMaximumApplicationAddress; // 검색할 메모리의 끝 주소
    MEMORY_BASIC_INFORMATION mbi;

    std::vector<std::thread> threads;

    // 전체 메모리 주소 공간 검색
    while (address < maxAddress) {
        // 메모리 페이지의 정보를 가져옴
        if (VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi)) == sizeof(mbi)) {
            // 해당 페이지가 메모리에 커밋되어 있고, 읽을 수 있는 페이지일 경우
            if (mbi.State == MEM_COMMIT && (mbi.Protect == PAGE_READWRITE || mbi.Protect == PAGE_READONLY || mbi.Protect == PAGE_EXECUTE_READ)) {
                SIZE_T regionSize = mbi.RegionSize;
                std::vector<uint8_t> buffer = readProcessMemory(hProcess, mbi.BaseAddress, regionSize);

                if (!buffer.empty()) {
                    // 각 페이지에서 Boyer-Moore를 사용하여 시그니처 검색
                    threads.emplace_back([&, regionSize, buffer, mbi]() {
                        int result = boyerMooreSearch(buffer, signature, 0, buffer.size());
                        if (result != -1) {
                            // 실제 메모리 주소 계산
                            LPCVOID foundAddress = (LPCVOID)((uintptr_t)mbi.BaseAddress + result);
                            std::lock_guard<std::mutex> lock(foundMutex);
                            foundAddresses.push_back(foundAddress);
                        }
                        });
                }
            }
            // 다음 메모리 영역으로 이동
            address = (LPCVOID)((uintptr_t)mbi.BaseAddress + mbi.RegionSize);
        }
        else {
            // 메모리 정보를 가져오지 못하면 더 이상 검색할 수 없음
            break;
        }
    }

    // 모든 스레드가 작업을 마치기를 기다림
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    // 결과 출력
    if (!foundAddresses.empty()) {
        for (const auto& addr : foundAddresses) {
            std::cout << "Pattern found at address: " << addr << std::endl;
        }
    }
    else {
        std::cout << "Pattern not found." << std::endl;
    }
}