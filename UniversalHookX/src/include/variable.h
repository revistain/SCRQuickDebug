#ifndef VARIABLE_H
#define VARIABLE_H
#include <vector>
#include <string>
#include <cstring>
#include "signature.h"
#include <unordered_map>

class StringTable {
public:
    std::vector<std::string> str;
    StringTable(std::string& str_data);
};

class EUDVariable {
public:
    uint32_t address;
    uint32_t value; // size in eudarray
    uint32_t prev_value;
    std::string file_name;
    std::string func_name;
    std::string var_name;
    std::string cgfw_type;
    std::vector<uint32_t> additional_value;
    std::vector<char*> display_buf; // ^*^
    bool pinned;
    EUDVariable(StringTable& string, uint32_t _file_index, uint32_t _func_index, uint32_t _var_index, uint32_t _addr);
    EUDVariable(StringTable& string, uint32_t _file_index, uint32_t _func_index, uint32_t _var_index, uint32_t _addr, uint32_t size);
    ~EUDVariable( );
};

class Variables {
public:
	StringTable strtable;
    std::vector<EUDVariable> eudvars;
    std::vector<EUDVariable> eudgvars;
    std::vector<EUDVariable> eudarrs;
    std::vector<EUDVariable> eudgarrs;
	std::vector<std::string> Locations;
    std::unordered_map<std::string, std::vector<std::pair<std::string, std::vector<std::reference_wrapper<EUDVariable>>>>> file_map; // damn...
    std::unordered_map < std::string, std::vector<std::reference_wrapper<EUDVariable>>> func_map;
    uint32_t screenTL[2];
	

	Variables(
        std::string& str_data, std::string& var_data, std::string& gvar_data,
        std::string& arr_data, std::string& garr_data, std::string& mrgn_data
    );

	void update_value();
};

Variables init_variables();
#endif