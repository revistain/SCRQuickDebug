#include "debugger_main.h"
#include "../console/console.hpp"
//#include "mpq.h"

Variables getVariables() {
	try {
		//extractMPQ();
		init_signature();
		Variables vars = init_variables();
        /*
        for (int i = 0; i < vars.eudvars.size(); i++) {
			LOG("func %s\n", vars.strtable.func_str[vars.eudvars[i].func_index].c_str());
            LOG("var  %s\n", vars.strtable.var_str[vars.eudvars[i].var_index].c_str());
			LOG("addr 0x%08X\n", getBaseAddr() + vars.eudvars[i].address);
		}
		for (int i = 0; i < vars.Locations.size(); i++) {
            LOG("loc %d: %s\n", i + 1, vars.Locations[i].c_str());
		}
		while (1) {
			vars.update_value();
			for (auto& var : vars.eudvars) {
				std::cout << vars.strtable.func_str[var.func_index] << "@@" << vars.strtable.var_str[var.var_index] << ": \t0x" << std::hex
					<< var.value << std::endl;
			}
			system("pause");
		}
		while (1) {
			process_packets();
		}
        */
        return vars;
    }
	catch (const char* e) {
        LOG("ERROR on getVariables: %s", e);
        throw "error catch on getVariables";
        system("pause");
    }
}
