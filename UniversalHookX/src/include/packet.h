#ifndef PACKET_H
#define PACKET_H
#include "signature.h"
#include <utility>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>

typedef struct {
	uint16_t opCode	: 16;
	uint16_t count	: 16;
	uint32_t param1 : 32;
	uint32_t param2 : 32;
	uint32_t param3 : 32;
	uint32_t mask	: 32;
}Packet;

void process_packets();
#endif