#include "eud.h"

uint32_t unEPD(uint32_t epd) {
	return ((epd << 2) + 0x58A364);
}