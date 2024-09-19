#include "variable.h"

EUDVariable::EUDVariable(uint32_t _func_index, uint32_t _var_index, uint32_t _addr) :
	address(_addr), func_index(_func_index), var_index(_var_index), value(0) {}

StringTable::StringTable(std::string& func_data, std::string& var_data, std::string& loc_data) {
	size_t start = 0;
	for (size_t end = 0; end < func_data.size(); end++) {
		if (func_data[end] == '\0') {
			func_str.push_back(func_data.substr(start, end - start));
			start = end + 1;
		}
	}
	start = 0;
	for (size_t end = 0; end < var_data.size(); end++) {
		if (var_data[end] == '\0') {
			var_str.push_back(var_data.substr(start, end - start));
			start = end + 1;
		}
	}

	start = 0;
	for (size_t end = 0; end < loc_data.size(); end++) {
		if (loc_data[end] == '\0') {
			loc_str.push_back(loc_data.substr(start, end - start));
			start = end + 1;
		}
	}
}

Variables::Variables(std::string& func_data, std::string& var_data, std::string& loc_data,
                     std::string& vdata, std::string& mdata) : strtable(func_data, var_data, loc_data) {
    screenTL[0] = 0;
    screenTL[1] = 0;
    uint32_t func_idx = 0;
	uint32_t var_idx = 0;
	uint32_t var_addr = 0;
	const uint32_t base_addr = getBaseAddr();
	for (size_t i = 0; i < vdata.size(); i += 12) {
		std::memcpy(&func_idx, &vdata[i], sizeof(uint32_t));
		std::memcpy(&var_idx, &vdata[i + 4], sizeof(uint32_t));
		std::memcpy(&var_addr, &vdata[i + 8], sizeof(uint32_t));
		eudvars.emplace_back(func_idx, var_idx, base_addr+var_addr);
	}

	Locations.resize(255);
	uint32_t loc_idx = 0;
	uint32_t loc_str_idx = 0;
	for (size_t i = 0; i < mdata.size(); i += 8) {
		std::memcpy(&loc_str_idx, &mdata[i], sizeof(uint32_t));
		std::memcpy(&loc_idx, &mdata[i + 4], sizeof(uint32_t));
		if (loc_idx > 0xFF) throw "location index bigger than 256";
		std::cout << std::dec << "one:" << loc_idx << " two:" << loc_str_idx << std::endl;
		if (loc_str_idx >= strtable.loc_str.size()) Locations[loc_idx] = "";
		else Locations[loc_idx] = strtable.loc_str[loc_str_idx];
	}

     uint32_t map_path_addr = getMapPathAddr();
     mapPath = strread(map_path_addr, 260);
}

Variables init_variables() {
	const uint32_t func_table_addr = getFuncTableAddr();
	std::string section_name = strread(func_table_addr, 4);
	std::cout << "section: " << section_name << std::endl;
	if (section_name != "FUNS") { throw "cannot find FUNS"; }
	uint32_t section_size = dwread(func_table_addr + 4);
	std::string func_str = strread(func_table_addr+8, section_size);

	const uint32_t var_table_addr = getVarTableAddr();
	section_name = strread(var_table_addr, 4);
	std::cout << "section: " << section_name << std::endl;
	if (section_name != "VARS") { throw "cannot find VARS"; }
	section_size = dwread(var_table_addr + 4);
	std::string var_str = strread(var_table_addr + 8, section_size);

	const uint32_t mrgn_table_addr = getMRGNTableAddr();
	section_name = strread(mrgn_table_addr, 4);
	std::cout << "section: " << section_name << std::endl;
	if (section_name != "MRNS") { throw "cannot find MRNS"; }
	section_size = dwread(mrgn_table_addr + 4);
	std::string mrgn_str = strread(mrgn_table_addr + 8, section_size);

	const uint32_t var_data_table_addr = getVarDataAddr();
	section_name = strread(var_data_table_addr, 4);
	std::cout << "section: " << section_name << std::endl;
	if (section_name != "VARD") { throw "cannot find VARD"; }
	section_size = dwread(var_data_table_addr + 4);
	std::string var_data_str = strread(var_data_table_addr + 8, section_size);

	const uint32_t mrgn_data_table_addr = getMRGNDataAddr();
	section_name = strread(mrgn_data_table_addr, 4);
	std::cout << "section: " << section_name << std::endl;
	if (section_name != "MRND") { throw "cannot find MRND"; }
	section_size = dwread(mrgn_data_table_addr + 4);
	std::string mrgn_data_str = strread(mrgn_data_table_addr + 8, section_size);

	
	return Variables(func_str, var_str, mrgn_str, var_data_str, mrgn_data_str); // RVO
}

void Variables::update_value() {
	// update screen size
    uint32_t screen_addr = getScreenDataAddr();
    screenTL[0] = dwread(screen_addr);
    screenTL[1] = dwread(screen_addr + 4);

	// update eudvariable
	for (auto& var : eudvars) {
		uint32_t addr = var.address;
		var.value = dwread(addr);
	}
}