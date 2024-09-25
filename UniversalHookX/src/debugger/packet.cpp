#include "packet.h"
#include "signature.h"

Packet readPacket(uint32_t address) {
    Packet packet;
    MEMORY_BASIC_INFORMATION mbi;
    HANDLE hProcess = getProcessHandle();
    if(VirtualQueryEx(hProcess, (LPCVOID)address, &mbi, sizeof(mbi)) == sizeof(mbi)) {
        if (mbi.State == MEM_COMMIT && mbi.Protect != PAGE_NOACCESS && mbi.Protect != PAGE_GUARD) {
            SIZE_T bytesRead;
            if (ReadProcessMemory(hProcess, (LPCVOID)address, &packet, sizeof(Packet), &bytesRead) && bytesRead == sizeof(Packet)) {
                return packet; // RVO
            }
            else {
                throw "Error reading Packet";
            }
        }
    }
}

uint32_t recentPacketNumber = 0;
std::vector<Packet> read_packets() {
    const uint32_t packet_addr    = getPacketAddr();
    const uint8_t whoIsUsing      = bread(packet_addr);           // 0: idle, 1: SC:R, 2: THIS
    const uint8_t isConnected     = bread(packet_addr + 1);       // bool
    const uint16_t stackedCount   = wread(packet_addr + 2);
    const uint32_t currentHead    = dwread(packet_addr + 4) << 2; // saved as EPD
    const uint32_t currentTail    = dwread(packet_addr + 8) << 2; // saved as EPD
    const uint32_t headerSize     = 12;

    std::cout << std::hex << "packet_addr: 0x" << packet_addr << std::endl;
    bwrite(packet_addr+1, 1);
    std::vector<Packet> receivedPackets;
    if (whoIsUsing == 0 && currentHead != currentTail) bwrite(packet_addr, 2); // FIX ME: SINCE IT DOESNT WORK SYNCRONIZEDLY, for now just let it be
    else return receivedPackets;

    if (stackedCount <= 50) {
        for (size_t i = 0; i < std::abs(static_cast<int>(currentTail) - static_cast<int>(currentHead)); i += 20) {
            uint32_t val = packet_addr + headerSize + ((currentHead + i) % 1000);
            Packet packet = readPacket(packet_addr + headerSize + ((currentHead + i) % 1000));
            receivedPackets.push_back(std::move(packet));
        }
    }
    else {
        for (size_t i = 0; i < 1000; i += 20) {
            Packet packet = readPacket(packet_addr + headerSize + ((currentHead + i) % 1000));
            receivedPackets.push_back(std::move(packet));
        }
    }

    dwwrite(packet_addr + 4, currentTail >> 2);
    bwrite(packet_addr, 0); // FIX ME
    return receivedPackets; 
}

void process_packets() {
    std::vector<Packet> packets = read_packets();
    for (auto& packet : packets) {
        std::cout << std::hex << "opcode\t: " << packet.opCode << std::endl;
        std::cout << std::hex << "count \t: " << packet.count << std::endl;
        std::cout << std::hex << "param1\t: " << packet.param1 << std::endl;
        std::cout << std::hex << "param2\t: " << packet.param2 << std::endl;
        std::cout << std::hex << "param3\t: " << packet.param3 << std::endl;
        std::cout << std::hex << "mask  \t: " << packet.mask << std::endl;

        switch (packet.opCode) {
        case 0:
            // pass
            break;
        case 1: // get initial datas
            
            break;
        case 2: // reply
            // param1: unique identifier
            // param2: value
            // param3: string offset
            break;
            
        }
    }
}


std::vector<Packet> send_packet_buf;
void addPacket(uint16_t opcode, uint16_t pNumber, uint32_t param1, uint32_t param2, uint32_t param3, uint32_t mask) {
    Packet packet;
    packet.opCode = opcode;
    packet.count = pNumber;
    packet.param1 = param1;
    packet.param2 = param2;
    packet.param3 = param3;
    packet.mask = mask;

    send_packet_buf.push_back(packet);
}

//// packet
// opcode 2byte
// pNumber 2bytes
// param1 4byte
// param2 4byte
// param3 4byte
// mask 4byte
void sendPacket(uint32_t offset, Packet& sending_packet) {
    MEMORY_BASIC_INFORMATION mbi;
    HANDLE hProcess = getProcessHandle( );

    if (VirtualQueryEx(hProcess, (LPCVOID)offset, &mbi, sizeof(mbi)) == sizeof(mbi)) {
        if (mbi.State == MEM_COMMIT && mbi.Protect != PAGE_NOACCESS && mbi.Protect != PAGE_GUARD) {
            Packet packet;            
            packet.opCode = sending_packet.opCode;
            packet.count = sending_packet.count;
            packet.param1 = sending_packet.param1;
            packet.param2 = sending_packet.param2;
            packet.param3 = sending_packet.param3;
            packet.mask = sending_packet.mask;

            SIZE_T bytesWritten;
            if (WriteProcessMemory(hProcess, (LPVOID)offset, &packet, sizeof(Packet), &bytesWritten) && bytesWritten == sizeof(Packet)) {
                // 성공적으로 씀
            } else {
                throw "Error writing Packet";
            }
        }
    }
}

//// receive packet header
// whoIsUsing 1byte
// isConnected 1byte
// stackedCount 2byte
// unused 4byte

const uint32_t send_Db_size = 1012;
bool send_packets( ) {
    if (send_packet_buf.size( ) == 0) return false;
    const uint32_t packet_addr = getPacketAddr( ) + send_Db_size;
    const uint8_t whoIsUsing = bread(packet_addr);          // 0: idle, 1: SC:R, 2: THIS
    const uint8_t isConnected = bread(packet_addr + 1);     // bool
    uint16_t stackedCount = wread(packet_addr + 2);
    const uint32_t headerSize = 4;

    std::cout << std::hex << "packet_addr: 0x" << packet_addr << std::endl;
    bwrite(packet_addr + 1, 1);
    std::vector<Packet> receivedPackets;
    if (whoIsUsing == 0) bwrite(packet_addr, 2); // FIX ME: SINCE IT DOESNT WORK SYNCRONIZEDLY, for now just let it be
    else return false;

    for (auto& packet : send_packet_buf) {
        if (stackedCount > 1000) {
            std::cout << "packet exceed!!\n";
            break;
        }
        sendPacket(packet_addr + stackedCount + headerSize, packet);
        stackedCount += 20;
    }
    send_packet_buf.clear( );

    std::cout << "stackCount: " << std::dec << stackedCount << "\n";
    wwrite(packet_addr + 2, stackedCount);
    bwrite(packet_addr, 0); // FIX ME
    return true;
}










