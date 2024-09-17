#include "signature.h"
#include "variable.h"
#include "packet.h"
#include "mpq.h"

int main() {
	try {
		extractMPQ();
		init_signature();
		Variables vars = init_variables();
		for (int i = 0; i < vars.eudvars.size(); i++) {
			std::cout << "func: " << vars.strtable.func_str[vars.eudvars[i].func_index] <<
				"\tvar: " << vars.strtable.var_str[vars.eudvars[i].var_index] <<
				"\taddr: 0x" << std::hex << getBaseAddr() + vars.eudvars[i].address << std::endl;
		}

		while (1) {
			vars.update_value();
			for (auto& var : vars.eudvars) {
				std::cout << vars.strtable.var_str[var.var_index] << ": \t0x" << std::hex
					<< var.value << std::endl;
			}
			system("pause");
		}
	}
	catch (const char* e) {
		std::cout << e << std::endl;
	}
	Packet packet;
	//readPacket(packet, getBaseAddr() + getPacketAddr() + 12);
}