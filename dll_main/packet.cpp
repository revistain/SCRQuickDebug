#include "packet.h"

void readPacket(Packet& packet, uint32_t address) {
    // Enumerate process memory regions
    MEMORY_BASIC_INFORMATION mbi;
    HANDLE hProcess = getProcessHandle();
    if(VirtualQueryEx(hProcess, (LPCVOID)address, &mbi, sizeof(mbi)) == sizeof(mbi)) {
        if (mbi.State == MEM_COMMIT && mbi.Protect != PAGE_NOACCESS && mbi.Protect != PAGE_GUARD) {
            // Only scan readable regions
            Packet packet;
            SIZE_T bytesRead;
            if (ReadProcessMemory(hProcess, (LPCVOID)address, &packet, sizeof(Packet)*5, &bytesRead)) {
                return;
            }
        }
    }
}
