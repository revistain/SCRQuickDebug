#include "memoryRW.h"

static HANDLE hProcess = NULL;
uint32_t exeAddr = 0;
uint32_t getEXEAddr() { return exeAddr; }
uintptr_t GetModuleBaseAddress(DWORD pid, const wchar_t* modName) {
    uintptr_t modBaseAddr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (hSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry)) {
            do {
                if (!_wcsicmp(modEntry.szModule, modName)) {
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    }
    CloseHandle(hSnap);
    return modBaseAddr;
}

bool _OpenTargetProcess(DWORD processID) {
    hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, processID);
    if (hProcess == NULL) {
        throw "Failed to open process";
        return false;
    }
    exeAddr = static_cast<uint32_t>(GetModuleBaseAddress(processID, L"StarCraft.exe"));
    return true;
}

bool OpenTargetProcess( ) {
    std::wstring processName = L"StarCraft.exe";
    DWORD processID = 0;

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
    if (!_OpenTargetProcess(processID)) {
        throw "error finding exe address";
    }
}

bool CloseTargetProcess( ) {
    CloseHandle(hProcess);
}

uint32_t findMRGNAddr(std::string map_path) {
    std::vector<uint8_t> signature = StringToByteVector(map_path);
    std::vector<uint32_t> foundAddresses = Internal::searchMemory(hProcess, signature);
    try {
        uint32_t mrgn_base_addr = Internal::choosePathAddr(foundAddresses);
        std::cout << "MRGN FOUND AT 0x" << std::hex << mrgn_base_addr << "\n";
        return mrgn_base_addr;
    } catch (const char* e) {
        throw e;
    }
}

namespace Internal {
    bool IsAddressAccessible(LPCVOID address, SIZE_T length) {
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(address, &mbi, sizeof(mbi))) {
            return (mbi.State == MEM_COMMIT) && (mbi.Protect & (PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE));
        }
        return false;
    }

    bool ReadMemory(LPCVOID address, SIZE_T length, BYTE* buffer) {
        if (!IsAddressAccessible(address, length)) {
            return false;
        }

        SIZE_T bytesRead;
        return ReadProcessMemory(hProcess, address, buffer, length, &bytesRead) && bytesRead == length;
    }

    bool WriteMemory(LPVOID address, SIZE_T length, BYTE* buffer) {
        if (!IsAddressAccessible(address, length)) {
            return false;
        }

        SIZE_T bytesWrite;
        return WriteProcessMemory(hProcess, address, buffer, length, &bytesWrite) && bytesWrite == length;
    }
    std::string strread(uint32_t address, size_t size) {
        std::vector<char> buf(size);
        if (ReadMemory(reinterpret_cast<LPCVOID>(address), size, reinterpret_cast<BYTE*>(buf.data( )))) {
            return std::string(buf.data( ), size);
        } else {
            std::cout << "Error: Failed to read memory from address " << address << ".\n";
            return std::string( );
        }
    }

    uint32_t dwread(uint32_t address) {
        uint32_t value = 0;
        if (ReadMemory(reinterpret_cast<LPCVOID>(address), sizeof(value), reinterpret_cast<BYTE*>(&value))) {
            return value;
        } else {
            std::cout << "Error: Failed to read 4bytes from memory.\n";
            return 0;
        }
    }

    uint16_t wread(uint32_t address) {
        uint16_t value = 0;
        if (ReadMemory(reinterpret_cast<LPCVOID>(address), sizeof(value), reinterpret_cast<BYTE*>(&value))) {
            return value;
        } else {
            std::cout << "Error: Failed to read 2bytes from memory.\n";
            return 0;
        }
    }

    uint8_t bread(uint32_t address) {
        uint8_t value = 0;
        if (ReadMemory(reinterpret_cast<LPCVOID>(address), sizeof(value), reinterpret_cast<BYTE*>(&value))) {
            return value;
        } else {
            std::cout << "Error: Failed to read byte from memory.\n";
            return 0;
        }
    }

    bool dwwrite(uint32_t address, uint32_t value) {
        if (WriteMemory(reinterpret_cast<LPVOID>(address), 4, reinterpret_cast<BYTE*>(&value))) {
            return true;
        } else {
            std::cout << "Error: Failed to write 4bytes to memory.\n";
            return false;
        }
    }

    bool wwrite(uint32_t address, uint16_t value) {
        if (WriteMemory(reinterpret_cast<LPVOID>(address), 2, reinterpret_cast<BYTE*>(&value))) {
            return true;
        } else {
            std::cout << "Error: Failed to write 2bytes to memory.\n";
            return false;
        }
    }

    bool bwrite(uint32_t address, uint8_t value) {
        if (WriteMemory(reinterpret_cast<LPVOID>(address), 1, reinterpret_cast<BYTE*>(&value))) {
            return true;
        } else {
            std::cout << "Error: Failed to write byte to memory.\n";
            return false;
        }
    }

    bool vecwrite(uint32_t address, std::vector<uint8_t> value) {
        if (WriteMemory(reinterpret_cast<LPVOID>(address), value.size( ), reinterpret_cast<BYTE*>(&value))) {
            return true;
        } else {
            std::cout << "Error: Failed to write vec to memory.\n";
            return false;
        }
    }

    uint32_t choosePathAddr(const std::vector<uint32_t> foundaddresses) {
        std::vector<uint32_t> maybe;
        for (auto& addr : foundaddresses) {
            if ((addr & 0x0C54) == 0x0C54) {
                maybe.push_back(addr);
            }
        }

        if (maybe.size( ) > 1) {
            throw "bad luck or memory layout of SC:R changed";
        }
        return maybe[0] + 0xDF24;
    }

    std::vector<uint32_t> searchMemory(HANDLE hProcess, std::vector<uint8_t>& signature) {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        std::vector<uint32_t> foundAddresses;

        LPCVOID address = sysInfo.lpMinimumApplicationAddress;
        LPCVOID maxAddress = reinterpret_cast<LPCVOID>(0xFFFFFFFF);
        MEMORY_BASIC_INFORMATION mbi;
        std::cout << std::hex << "start address: 0x" << address << "  end: 0x" << maxAddress << "\n";

        buildBadCharTable(signature);
        while (address < maxAddress) {
            if (VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi)) == sizeof(mbi)) {
                if (mbi.State == MEM_COMMIT &&
                    (mbi.Protect == PAGE_READWRITE ||
                     mbi.Protect == PAGE_READONLY ||
                     mbi.Protect == PAGE_EXECUTE_READ)) {

                    SIZE_T regionSize = mbi.RegionSize;
                    std::vector<uint8_t> buffer = readProcessMemory(hProcess, mbi.BaseAddress, regionSize);

                    if (!buffer.empty( )) {
                        int result = boyerMooreSearch(buffer, signature, 0, buffer.size( ));
                        if (result != -1) {
                            LPCVOID foundAddress = (LPCVOID)((uintptr_t)mbi.BaseAddress + result);

                            foundAddresses.push_back(reinterpret_cast<uint32_t>(foundAddress));
                        }
                    }
                }
                address = (LPCVOID)((uintptr_t)mbi.BaseAddress + mbi.RegionSize);
            }
            else {
                break;
            }
        }
        return foundAddresses;
    }
} // namespace Internal