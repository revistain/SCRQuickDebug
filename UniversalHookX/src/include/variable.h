#ifndef VARIABLE_H
#define VARIABLE_H
#include <vector>
#include <string>
#include <cstring>
#include "signature.h"

class EUDVariable {
public:
	uint32_t address;
	uint32_t func_index;
	uint32_t var_index;
	uint32_t value;
	EUDVariable(uint32_t _func_index, uint32_t _var_index, uint32_t _addr);
};

class StringTable {
public:
	std::vector<std::string> func_str;
	std::vector<std::string> var_str;
	std::vector<std::string> loc_str;
	StringTable(std::string& func_data, std::string& var_data, std::string& loc_data);
};

class Variables {
public:
	StringTable strtable;
	std::vector<EUDVariable> eudvars;
	std::vector<std::string> Locations;
    uint32_t screenTL[2];
    std::string mapPath;
	

	Variables(std::string& func_data, std::string& var_data,
		std::string& loc_data, std::string& vdata, std::string& mdata);
	void update_value();
};

Variables init_variables();
#endif