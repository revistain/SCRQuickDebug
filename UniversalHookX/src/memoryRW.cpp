#include "memoryRW.h"


uint32_t findMRGNAddr(std::string map_path) {
    std::vector<uint8_t> signature = StringToByteVector(map_path);
    std::vector<uint32_t> foundAddresses = Internal::searchMemory(getProcessHandle( ), signature);
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
        return ReadProcessMemory(getProcessHandle( ), address, buffer, length, &bytesRead) && bytesRead == length;
    }

    bool WriteMemory(LPVOID address, SIZE_T length, BYTE* buffer) {
        if (!IsAddressAccessible(address, length)) {
            return false;
        }

        SIZE_T bytesWrite;
        return WriteProcessMemory(getProcessHandle( ), address, buffer, length, &bytesWrite) && bytesWrite == length;
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

    std::vector<uint8_t> vecread(uint32_t address, size_t size) {
        std::vector<uint8_t> buf(size);
        if (ReadMemory(reinterpret_cast<LPCVOID>(address), size, reinterpret_cast<BYTE*>(buf.data( )))) {
            return buf;
        } else {
            std::cout << "Error: Failed to read memory from address " << address << ".\n";
            return std::vector<uint8_t>();
        }
    }

    uint32_t dwread(uint32_t address) {
        uint32_t value = 0;
        if (ReadMemory(reinterpret_cast<LPCVOID>(address), sizeof(value), reinterpret_cast<BYTE*>(&value))) {
            return value;
        } else {
            std::cout << "Error: Failed to read 0x" << std::hex << address << std::dec << " 4bytes from memory.\n";
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
        // TODO: find this when finding signature, dont scan twice
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        std::vector<uint32_t> foundAddresses;

        LPCVOID address = sysInfo.lpMinimumApplicationAddress;
        LPCVOID maxAddress = reinterpret_cast<LPCVOID>(0x7FFFFFFF);
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