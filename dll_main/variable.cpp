#include "variable.h"

EUDVariable::EUDVariable(uint32_t _func_index, uint32_t _var_index, uint32_t _addr) :
	address(_addr), func_index(_func_index), var_index(_var_index), value(0) {}

StringTable::StringTable(std::string& func_data, std::string& var_data) {
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
}

Variables::Variables(std::string& func_data, std::string& var_data, std::string& data) : strtable(func_data, var_data) {
	uint32_t func_idx = 0;
	uint32_t var_idx = 0;
	uint32_t var_addr = 0;
	for (size_t i = 0; i < data.size(); i += 12) {
		std::memcpy(&func_idx, &data[i], sizeof(uint32_t));
		std::memcpy(&var_idx, &data[i + 4], sizeof(uint32_t));
		std::memcpy(&var_addr, &data[i + 8], sizeof(uint32_t));

		eudvars.emplace_back(func_idx, var_idx, var_addr);
	}
}

Variables init_variables() {
	const uint32_t base_addr = getBaseAddr();

	const uint32_t func_table_addr = base_addr + getFuncTableAddr();
	std::string section_name = strread(func_table_addr, 4);
	std::cout << "section: " << section_name << std::endl;
	if (section_name != "FUNS") { throw "cannot find FUNS"; }
	uint32_t section_size = dwread(func_table_addr + 4);
	std::string func_str = strread(func_table_addr+8, section_size);

	const uint32_t var_table_addr = base_addr + getVarTableAddr();
	section_name = strread(var_table_addr, 4);
	std::cout << "section: " << section_name << std::endl;
	if (section_name != "VARS") { throw "cannot find VARS"; }
	section_size = dwread(var_table_addr + 4);
	std::string var_str = strread(var_table_addr + 8, section_size);

	const uint32_t var_data_table_addr = base_addr + getVarDataAddr();
	section_name = strread(var_data_table_addr, 4);
	std::cout << "section: " << section_name << std::endl;
	if (section_name != "VARD") { throw "cannot find VARD"; }
	section_size = dwread(var_data_table_addr + 4);
	std::string var_data_str = strread(var_data_table_addr + 8, section_size);

	return std::move(Variables(func_str, var_str, var_data_str));
}

void Variables::update_value() {
	const uint32_t base_addr = getBaseAddr();
	for (auto& var : eudvars) {
		uint32_t addr = base_addr + var.address;
		var.value = dwread(addr);
	}
}