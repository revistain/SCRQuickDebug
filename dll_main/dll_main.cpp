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
		for (int i = 0; i < vars.Locations.size(); i++) {
			if(vars.Locations[i] != "") std::cout << std::dec << "loc " << i + 1 << ": " << vars.Locations[i] << std::endl;
		}

		/*
		while (1) {
			vars.update_value();
			for (auto& var : vars.eudvars) {
				std::cout << vars.strtable.func_str[var.func_index] << "@@" << vars.strtable.var_str[var.var_index] << ": \t0x" << std::hex
					<< var.value << std::endl;
			}
			system("pause");
		}
		*/
		while (1) {
			process_packets();
			system("pause");
		}
	}
	catch (const char* e) {
		std::cout << e << std::endl;
	}
}