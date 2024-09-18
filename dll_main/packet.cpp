#include "packet.h"
#include "signature.h"

void readPacket(Packet& packet, uint32_t address) {
    // Enumerate process memory regions
    MEMORY_BASIC_INFORMATION mbi;
    HANDLE hProcess = getProcessHandle();
    if(VirtualQueryEx(hProcess, (LPCVOID)address, &mbi, sizeof(mbi)) == sizeof(mbi)) {
        if (mbi.State == MEM_COMMIT && mbi.Protect != PAGE_NOACCESS && mbi.Protect != PAGE_GUARD) {
            // Only scan readable regions
            SIZE_T bytesRead;
            if (ReadProcessMemory(hProcess, (LPCVOID)address, &packet, sizeof(Packet)*5, &bytesRead) && bytesRead == sizeof(Packet) * 5) {
                return;
            }
            else {
                throw "Error reading Packet";
            }
        }
    }
}


void update_packet() {
    uint32_t packet_addr    = getPacketAddr();
    uint16_t whoIsUsing     = wread(packet_addr);
    uint16_t isConnected    = wread(packet_addr + 2);
    uint32_t currentHeadEPD = wread(packet_addr + 4);
    uint32_t currentEndEPD  = wread(packet_addr + 8) << 2;



}