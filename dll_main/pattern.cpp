#include "pattern.h"
#include <iostream>
#include <vector>
#include <thread>
#include <windows.h>
#include <mutex>

// Mutex for safe access to foundAddresses
std::mutex foundMutex;
std::vector<LPCVOID> foundAddresses; // ������ �޸� �ּ�

// Boyer-Moore�� bad character ���̺� ����
std::vector<int> buildBadCharTable(const std::vector<uint8_t>& needle) {
    const int ALPHABET_SIZE = 256;
    std::vector<int> badCharTable(ALPHABET_SIZE, -1);

    for (int i = 0; i < needle.size(); i++) {
        badCharTable[needle[i]] = i;
    }

    return badCharTable;
}

// Boyer-Moore �˰���
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
            return shift; // ������ ã���� ���� ���� ��ġ
        }
        else {
            shift += max(1, j - badCharTable[haystack[shift + j]]);
        }
    }

    return -1; // ������ ã�� ���� ���
}

// �޸𸮿��� ���μ��� �б�
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

// �޸� ������ ������ �˻��ϴ� �Լ�
void searchMemory(HANDLE hProcess, const std::vector<uint8_t>& signature, int numThreads) {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo); // �ý��� ���� ��������

    LPCVOID address = sysInfo.lpMinimumApplicationAddress; // �˻��� �޸��� ���� �ּ�
    LPCVOID maxAddress = sysInfo.lpMaximumApplicationAddress; // �˻��� �޸��� �� �ּ�
    MEMORY_BASIC_INFORMATION mbi;

    std::vector<std::thread> threads;

    // ��ü �޸� �ּ� ���� �˻�
    while (address < maxAddress) {
        // �޸� �������� ������ ������
        if (VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi)) == sizeof(mbi)) {
            // �ش� �������� �޸𸮿� Ŀ�ԵǾ� �ְ�, ���� �� �ִ� �������� ���
            if (mbi.State == MEM_COMMIT && (mbi.Protect == PAGE_READWRITE || mbi.Protect == PAGE_READONLY || mbi.Protect == PAGE_EXECUTE_READ)) {
                SIZE_T regionSize = mbi.RegionSize;
                std::vector<uint8_t> buffer = readProcessMemory(hProcess, mbi.BaseAddress, regionSize);

                if (!buffer.empty()) {
                    // �� ���������� Boyer-Moore�� ����Ͽ� �ñ״�ó �˻�
                    threads.emplace_back([&, regionSize, buffer, mbi]() {
                        int result = boyerMooreSearch(buffer, signature, 0, buffer.size());
                        if (result != -1) {
                            // ���� �޸� �ּ� ���
                            LPCVOID foundAddress = (LPCVOID)((uintptr_t)mbi.BaseAddress + result);
                            std::lock_guard<std::mutex> lock(foundMutex);
                            foundAddresses.push_back(foundAddress);
                        }
                        });
                }
            }
            // ���� �޸� �������� �̵�
            address = (LPCVOID)((uintptr_t)mbi.BaseAddress + mbi.RegionSize);
        }
        else {
            // �޸� ������ �������� ���ϸ� �� �̻� �˻��� �� ����
            break;
        }
    }

    // ��� �����尡 �۾��� ��ġ�⸦ ��ٸ�
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    // ��� ���
    if (!foundAddresses.empty()) {
        for (const auto& addr : foundAddresses) {
            std::cout << "Pattern found at address: " << addr << std::endl;
        }
    }
    else {
        std::cout << "Pattern not found." << std::endl;
    }
}