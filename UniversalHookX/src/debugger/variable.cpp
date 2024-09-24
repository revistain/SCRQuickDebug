#include "variable.h"
#include "eud.h"

void EUDVariable::updateDb() {
    if (cgfw_type != "StringBuffer" && cgfw_type != "Db") return;
    uint32_t base_addr = getBaseAddr( );
    if (cgfw_type == "StringBuffer") {
        uint32_t addr = unEPD(address);
        Db = strread(base_addr + addr, value);
    }
    else if (cgfw_type == "Db") {
        Db = strread(base_addr + address, value);
    }
}
        
EUDVariable::EUDVariable(StringTable& string, uint32_t _file_index, uint32_t _func_index,
    uint32_t _var_index, uint32_t _addr) : address(_addr), value(0), prev_value(0), cgfw_type(""), watchingDb(false) {
    file_name = string.str[_file_index];
    if (_func_index == -1) { func_name = ""; }
    else { func_name = string.str[_func_index]; }
    var_name  = string.str[_var_index];
    display_buf.resize(1);
    pinned = false;
    std::cout << "var: " << file_name << "@" << func_name << "@@" << var_name << "\n";
}

// init with size
EUDVariable::EUDVariable(StringTable& string, uint32_t _file_index, uint32_t _func_index,
                         uint32_t _var_index, uint32_t _addr, uint32_t _size) : address(_addr), value(_size), prev_value(0), cgfw_type(""), watchingDb(false) {
    file_name = string.str[_file_index];
    if (_func_index == -1) { func_name = ""; }
    else { func_name = string.str[_func_index]; }
    var_name = string.str[_var_index];
    display_buf.resize(_size);
    pinned = false;
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
    for (size_t i = 0; i < gvar_data.size( ); i += 12) {
        std::memcpy(&file_idx, &gvar_data[i], sizeof(uint32_t));
        std::memcpy(&var_idx, &gvar_data[i + 4], sizeof(uint32_t));
        std::memcpy(&addr, &gvar_data[i + 8], sizeof(uint32_t));
        eudgvars.emplace_back(strtable, file_idx, -1, var_idx, base_addr + addr);
    }
    for (size_t i = 0; i < arr_data.size( ); i += 20) {
        std::memcpy(&file_idx, &arr_data[i], sizeof(uint32_t));
        std::memcpy(&func_idx, &arr_data[i + 4], sizeof(uint32_t));
        std::memcpy(&var_idx, &arr_data[i + 8], sizeof(uint32_t));
        std::memcpy(&addr, &arr_data[i + 12], sizeof(uint32_t)); // EUDArray pass epd from euddraft 0.1.0.0
        std::memcpy(&size, &arr_data[i + 16], sizeof(uint32_t));
        eudarrs.emplace_back(strtable, file_idx, func_idx, var_idx, base_addr + unEPD(addr), size);
        eudarrs[eudarrs.size( ) - 1].cgfw_type = "EUDArray";
        eudarrs[eudarrs.size( ) - 1].additional_value.resize(size);
    }
    for (size_t i = 0; i < garr_data.size( ); i += 20) {
        std::memcpy(&file_idx, &garr_data[i], sizeof(uint32_t));
        std::memcpy(&func_idx, &garr_data[i + 4], sizeof(uint32_t));
        std::memcpy(&var_idx, &garr_data[i + 8], sizeof(uint32_t));
        std::memcpy(&addr, &garr_data[i + 12], sizeof(uint32_t));
        std::memcpy(&size, &garr_data[i + 16], sizeof(uint32_t));
        eudgarrs.emplace_back(strtable, file_idx, -1, var_idx, addr, size);
        eudgarrs.back().cgfw_type = strtable.str[func_idx];
        eudgarrs.back().additional_value.resize(size);
    }

	Locations.resize(255);
	uint32_t loc_str_idx = 0;
	uint32_t loc_idx = 0;
    for (size_t i = 0; i < strtable.str.size( ); i++) {
        // std::cout << i << "/ " << strtable.str[i] << "\n";
    }

    for (size_t i = 0; i < mrgn_data.size( ); i += 8) {
        std::memcpy(&loc_str_idx, &mrgn_data[i], sizeof(uint32_t));
        std::memcpy(&loc_idx, &mrgn_data[i + 4], sizeof(uint32_t));
        Locations[loc_idx] = strtable.str[loc_str_idx];
	}
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
    if (section_name != "MRNT") {
        throw "cannot find MRNT";
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

    uint32_t base_addr = getBaseAddr( );
    uint32_t idx = 0;
    for (auto& arr : eudgarrs) {
        if (arr.cgfw_type == "EUDArray") {
            uint32_t addr = unEPD(arr.address);
            for (size_t i = 0; i < arr.value; i++) {
                arr.additional_value[i] = dwread(base_addr +addr + i * 4);
            }
        }
        if (arr.cgfw_type == "PVariable") {
            uint32_t addr = arr.address;
            for (size_t i = 0; i < arr.value; i++) {
                arr.additional_value[i] = dwread(base_addr + addr + i * 72 + 348);
            }
        }
        if (arr.cgfw_type == "StringBuffer" || arr.cgfw_type == "Db") {
            if (arr.watchingDb && !arr.mem_edit.Open) {
                arr.watchingDb = false;
            }
            else if (arr.watchingDb) {
                if (arr.func_name == "") {
                    arr.mem_edit.DrawWindow(std::format("Db Viewer [ {} / Global / {} ]##{}", arr.file_name, arr.var_name, idx).c_str( ), arr.Db.data( ), arr.value);
                }
                else {
                    arr.mem_edit.DrawWindow(std::format("Db Viewer [ {} / {} / {} ]##{}", arr.file_name, arr.func_name, arr.var_name, idx).c_str( ), arr.Db.data( ), arr.value);
                }
            }
        }
        idx++;
    }
    for (auto& arr : eudgvars) {
        uint32_t addr = arr.address;
        uint32_t temp = arr.value;
        arr.value = dwread(addr);
        if (temp != arr.value) { arr.prev_value = temp; }
    }
    for (auto& arr : eudarrs) {
        uint32_t addr = arr.address;
        for (size_t i = 0; i < arr.value; i++) {
            arr.additional_value[i] = dwread(addr + i * 4);
        }
        
    }
    for (auto& arr : eudvars) {
        uint32_t addr = arr.address;
        uint32_t temp = arr.value;
        arr.value = dwread(addr);
        if (temp != arr.value) { arr.prev_value = temp; }
    }
}
