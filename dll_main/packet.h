#ifndef PACKET_H
#define PACKET_H
#include "signature.h"
#include <utility>
#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <string>

typedef struct {
	uint16_t opCode	: 2;
	uint16_t count	: 2;
	uint32_t param1 : 4;
	uint32_t param2 : 4;
	uint32_t param3 : 4;
	uint32_t mask	: 4;
}Packet;

std::vector<Packet> packets;

void readPacket(Packet& packet, uint32_t address);
#endif