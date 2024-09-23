#include "variable.h"

        
EUDVariable::EUDVariable(StringTable& string, uint32_t _file_index, uint32_t _func_index,
    uint32_t _var_index, uint32_t _addr) : address(_addr), value(0) {
    file_name = string.str[_file_index];
    func_name = string.str[_func_index];
    var_name  = string.str[_var_index];

}

StringTable::StringTable(std::string& str_data) {
	size_t start = 0;
    for (size_t end = 0; end < str_data.size( ); end++) {
            if (str_data[end] == '\0') {
            str.push_back(str_data.substr(start, end - start));
			start = end + 1;
		}
	}
}

Variables::Variables(
    std::string& str_data, std::string& var_data, std::string& gvar_data,
    std::string& arr_data, std::string& garr_data, std::string& mrgn_data
) : strtable(str_data) {
    screenTL[0] = 0;
    screenTL[1] = 0;
    uint32_t file_idx = 0;
    uint32_t func_idx = 0;
    uint32_t var_idx = 0;
    uint32_t addr = 0;
    uint32_t size = 0;
	const uint32_t base_addr = getBaseAddr();
    for (size_t i = 0; i < var_data.size( ); i += 16) {
		std::memcpy(&file_idx, &var_data[i], sizeof(uint32_t));
		std::memcpy(&func_idx, &var_data[i + 4], sizeof(uint32_t));
		std::memcpy(&var_idx, &var_data[i + 8], sizeof(uint32_t));
		std::memcpy(&addr, &var_data[i + 12], sizeof(uint32_t));
		eudvars.emplace_back(strtable, file_idx, func_idx, var_idx, base_addr+addr);
	}

    for (size_t i = 0; i < gvar_data.size( ); i += 16) {
        std::memcpy(&file_idx, &gvar_data[i], sizeof(uint32_t));
        std::memcpy(&var_idx, &gvar_data[i + 4], sizeof(uint32_t));
        std::memcpy(&addr, &gvar_data[i + 8], sizeof(uint32_t));
        eudgvars.emplace_back(strtable, file_idx, 0, var_idx, base_addr + addr);
    }

    for (size_t i = 0; i < arr_data.size( ); i += 16) {
        std::memcpy(&file_idx, &arr_data[i], sizeof(uint32_t));
        std::memcpy(&func_idx, &arr_data[i + 4], sizeof(uint32_t));
        std::memcpy(&var_idx, &arr_data[i + 8], sizeof(uint32_t));
        std::memcpy(&addr, &arr_data[i + 12], sizeof(uint32_t));
        std::memcpy(&size, &arr_data[i + 16], sizeof(uint32_t));
        eudarrs.emplace_back(strtable, file_idx, func_idx, var_idx, base_addr + addr, size);
    }

    for (size_t i = 0; i < garr_data.size( ); i += 16) {
        std::memcpy(&file_idx, &arr_data[i], sizeof(uint32_t));
        std::memcpy(&func_idx, &arr_data[i + 4], sizeof(uint32_t));
        std::memcpy(&var_idx, &arr_data[i + 8], sizeof(uint32_t));
        std::memcpy(&addr, &arr_data[i + 12], sizeof(uint32_t));
        std::memcpy(&size, &arr_data[i + 16], sizeof(uint32_t));
        eudgarrs.emplace_back(strtable, file_idx, 0, var_idx, base_addr + addr, size);
        eudgarrs[eudgarrs.size( ) - 1].cgfw_type = strtable.str[func_idx];
    }

	Locations.resize(255);
	uint32_t loc_str_idx = 0;
	uint32_t loc_idx = 0;
    for (size_t i = 0; i < mrgn_data.size( ); i += 8) {
        std::memcpy(&loc_idx, &mrgn_data[i], sizeof(uint32_t));
        std::memcpy(&loc_str_idx, &mrgn_data[i + 4], sizeof(uint32_t));
        if (strtable.str[loc_str_idx] == "")  Locations[loc_idx] = "";
        else Locations[loc_idx] = strtable.str[loc_str_idx];
	}

    /*
	 for (auto& var : eudvars) {
         bool pushed = false;
         for (auto& func : func_var) {
             std::string& func_string = func.first;
             if (func.first == var.func_name) {
			 	 func.second.push_back(var);
                 pushed = true;
			 	 break;
			 }
		 }
         if (!pushed) {
            std::vector<std::reference_wrapper<EUDVariable>> eudvarvec;
            eudvarvec.push_back(var);
            auto eudpair = std::pair<std::string, std::vector<std::reference_wrapper<EUDVariable>>>(strtable.func_str[var.func_index], eudvarvec);
            func_var.push_back(eudpair);
		 }
	 }
     */
}

Variables init_variables() {
	const uint32_t string_addr = getStringAddr();
    std::string section_name = strread(string_addr, 4);
	std::cout << "section: " << section_name << std::endl;
	if (section_name != "STRS") { throw "cannot find STRS"; }
    uint32_t section_size = dwread(string_addr + 4);
    std::string string_data = strread(string_addr + 8, section_size);

	const uint32_t var_addr = getVarAddr( );
    section_name = strread(var_addr, 4);
    std::cout << "section: " << section_name << std::endl;
    if (section_name != "VART") {
        throw "cannot find VART";
    }
    section_size = dwread(var_addr + 4);
    std::string var_data = strread(var_addr + 8, section_size);

	const uint32_t gvar_addr = getGVarAddr( );
    section_name = strread(gvar_addr, 4);
    std::cout << "section: " << section_name << std::endl;
    if (section_name != "GVRT") {
        throw "cannot find GVRT";
    }
    section_size = dwread(gvar_addr + 4);
    std::string gvar_data = strread(gvar_addr + 8, section_size);

	const uint32_t arr_addr = getArrAddr( );
    section_name = strread(arr_addr, 4);
    std::cout << "section: " << section_name << std::endl;
    if (section_name != "ARRT") {
        throw "cannot find ARRT";
    }
    section_size = dwread(arr_addr + 4);
    std::string arr_data = strread(arr_addr + 8, section_size);

	const uint32_t garr_addr = getGArrAddr( );
    section_name = strread(garr_addr, 4);
    std::cout << "section: " << section_name << std::endl;
    if (section_name != "GART") {
        throw "cannot find GART";
    }
    section_size = dwread(garr_addr + 4);
    std::string garr_data = strread(garr_addr + 8, section_size);

	const uint32_t mrgn_addr = getMRGNDataAddr( );
    section_name = strread(mrgn_addr, 4);
    std::cout << "section: " << section_name << std::endl;
    if (section_name != "MRND") {
        throw "cannot find MRND";
    }
    section_size = dwread(mrgn_addr + 4);
    std::string mrgn_data = strread(mrgn_addr + 8, section_size);

	return Variables::Variables(
        string_data, var_data, gvar_data,
        arr_data, garr_data, mrgn_data
    );
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