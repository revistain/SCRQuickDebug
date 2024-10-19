#ifndef VARIABLE_H
#define VARIABLE_H
#include <vector>
#include <string>
#include <cstring>
#include <format>
#include "../imgui_memory_editor.h"
#include <unordered_map>
#include "signature.h"
#include "unittable.h"

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
    std::string Db;

    std::vector<std::string> display_buf;
    bool pinned;
    MemoryEditor mem_edit;
    bool watchingDb;

    EUDVariable(StringTable& string, uint32_t _file_index, uint32_t _func_index, uint32_t _var_index, uint32_t _addr);
    EUDVariable(StringTable& string, uint32_t _file_index, uint32_t _func_index, uint32_t _var_index, uint32_t _addr, uint32_t size);
    void updateDb();
    //EUDVariable(const EUDVariable& eudvar);
    //~EUDVariable( );
};

typedef struct {
    uint32_t isSingle;
    uint32_t map_title_offset;
    uint32_t map_title_idx;
}WireFrame;

typedef struct {
    uint32_t stack_addr;
    uint32_t stackCount;
    uint32_t offset;
} FuncTrace;

class Variables {
public:
    StringTable strtable;
    std::vector<EUDVariable> eudvars;
    std::vector<EUDVariable> eudgvars;
    std::vector<EUDVariable> eudarrs;
    std::vector<EUDVariable> eudgarrs;
    std::vector<std::string> Locations;
    std::vector<bool> LocationsUse;
    std::unordered_map<std::string, std::vector<std::pair<std::string, std::vector<std::reference_wrapper<EUDVariable>>>>> file_map; // damn...
    std::unordered_map<std::string, std::vector<std::reference_wrapper<EUDVariable>>> func_map;
    uint32_t screenTL[2];
    uint32_t selectedUnit[12];

    WireFrame wfdata;
    FuncTrace functrace;


    Variables(
        std::string& str_data, std::string& var_data, std::string& gvar_data, std::string& arr_data,
        std::string& garr_data, std::string& mrgn_data, std::string& wf_data, std::string& functrace_data);
    //~Variables( );
    // Variables(const Variables& vars);

    void update_value( );
};
Variables init_variables( );
#endif