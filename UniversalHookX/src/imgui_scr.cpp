#include "imgui_scr.h"
#include "memoryRW.h"
#include "debugger_main.h"
#include "signature.h"
#include "font.h"
#include "imgui_things.h"
#include "console/console.hpp"
#include <format>
#include <memory>
#include <codecvt>
#include <locale>
#include <cstddef>

static bool detachFlag = false;
bool isExit( ) { return detachFlag; }
void setExit( ) { detachFlag = true; }

// Global variables
ID3D11Device* g_pd3dDevice = NULL;
ID3D11DeviceContext* g_pd3dDeviceContext = NULL;

std::unique_ptr<Variables> var_ptr;
std::unique_ptr<Locations> loc_ptr;
std::unique_ptr<CUnits>    unit_ptr;
void sortVariables(std::unique_ptr<Variables>& var_ptr) {
    for (auto& arr : var_ptr->eudgarrs) {
        bool flag = false;
        for (auto& file : var_ptr->file_map[arr.file_name]) {
            if (file.first == arr.func_name) {
                flag = true;
                file.second.push_back(std::ref(arr));
            }
        }
        if (!flag) {
            std::vector<std::reference_wrapper<EUDVariable>> obj;
            obj.push_back(std::ref(arr));
            var_ptr->file_map[arr.file_name].emplace_back(arr.func_name, obj);
        }
    }
    for (auto& arr : var_ptr->eudgvars) {
        bool flag = false;
        for (auto& file : var_ptr->file_map[arr.file_name]) {
            if (file.first == arr.func_name) {
                flag = true;
                file.second.push_back(std::ref(arr));
            }
        }
        if (!flag) {
            std::vector<std::reference_wrapper<EUDVariable>> obj;
            obj.push_back(std::ref(arr));
            var_ptr->file_map[arr.file_name].emplace_back(arr.func_name, obj);
        }
    }
    for (auto& arr : var_ptr->eudarrs) {
        bool flag = false;
        for (auto& file : var_ptr->file_map[arr.file_name]) {
            if (file.first == arr.func_name) {
                flag = true;
                file.second.push_back(std::ref(arr));
            }
        }
        if (!flag) {
            std::vector<std::reference_wrapper<EUDVariable>> obj;
            obj.push_back(std::ref(arr));
            var_ptr->file_map[arr.file_name].emplace_back(arr.func_name, obj);
        }
    }
    for (auto& arr : var_ptr->eudvars) {
        bool flag = false;
        for (auto& file : var_ptr->file_map[arr.file_name]) {
            if (file.first == arr.func_name) {
                flag = true;
                file.second.push_back(std::ref(arr));
            }
        }
        if (!flag) {
            std::vector<std::reference_wrapper<EUDVariable>> obj;
            obj.push_back(std::ref(arr));
            var_ptr->file_map[arr.file_name].emplace_back(arr.func_name, obj);
        }
    }

    for (auto& file : var_ptr->file_map) {
        std::cout << std::hex << "file name: " << file.first << "\n";
        for (auto& obj : file.second) {
            if (obj.first == "")
                std::cout << "- function: GLOBAL" << obj.first << "\n";
            else
                std::cout << "- function: " << obj.first << "\n";
            for (auto& var : obj.second) {
                if (var.get( ).cgfw_type != "") {
                    std::cout << "  - type : " << var.get( ).cgfw_type << "\n";
                }
                std::cout << "  - var : " << var.get( ).var_name << " / 0x" << var.get( ).address << "\n";
            }
        }
    }
}

bool writeWFData(std::unique_ptr<Variables>& var_ptr, uint32_t exe_addr) {
    if (var_ptr->wfdata.isSingle > 0) return false;
    uint32_t base_addr = exe_addr + 0xDDF0C8;
    std::string map_string = var_ptr->strtable.str[var_ptr->wfdata.map_title_idx];
    std::vector<uint8_t> map_vec(map_string.begin( ), map_string.end( ));
    std::cout << "map: " << map_string << "\n";
    std::cout << "offset: " << std::dec << var_ptr->wfdata.map_title_offset << "\n";
    for (size_t i = 0; i < map_vec.size( ); i++) {
        Internal::bwrite(base_addr + var_ptr->wfdata.map_title_offset + i, map_string[i]);
    }

    return true;
}


bool starcraft_input = true;
static bool is_var_popup_open = false;
static bool is_unit_popup_open = false;
GameData updateGameData( ) {
    // if game ended
    if (getElapsedTime(loc_ptr->mrgn_addr) == 0) {
        std::cout << "elapsed timer = 0\n";
        setExit( );
    }

    // from eudplib
    var_ptr->update_value( );

    // from process
    GameData gdata;
    uint32_t exeAddr = getEXEAddr( );
    gdata.screen_size_x  = Internal::dwread(exeAddr + 0xB31AC0);
    gdata.screen_size_y  = Internal::dwread(exeAddr + 0xB31AC8);
    gdata.console_height = Internal::dwread(exeAddr + 0xB31AC4); // unsued
    gdata.pillar_size    = Internal::dwread(exeAddr + 0xDE6104);
    loc_ptr->updateData( );
    unit_ptr->update( );
    return gdata;
}


void onImguiStart() {
    if (var_ptr) return;
    // from eudplib
    try {
        var_ptr = std::make_unique<Variables>(getVariables( ));
        sortVariables(var_ptr);

        // from process
        if (OpenTargetProcess( )) {
            setEXEAddr(GetModuleBaseAddress(L"StarCraft.exe"));
            // setUnittableAddrOpenGL(GetModuleBaseAddress(L"Opengl32.dll")); // deprecated
            setUnittableAddr(getEXEAddr());
            unit_ptr = std::make_unique<CUnits>(getUnittableAddr());
        }
        writeWFData(var_ptr, getEXEAddr( ));
        loc_ptr = std::make_unique<Locations>(findMRGNAddr( ), var_ptr->Locations);
    } catch (const std::string& str) {
        throw std::string("error on onImguiStart\n") + str;
    }
}

// Your own window and controls
void StarCraft_UI( ) {
    ImVec2 screenSize = ImGui::GetIO( ).DisplaySize;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_AlwaysAutoResize |
                                    ImGuiWindowFlags_NoBackground |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (starcraft_input) window_flags |= ImGuiWindowFlags_NoInputs;

    // Set the window size to full screen
    ImGui::SetNextWindowSize(screenSize);
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    ///////////////////////////////////////////////////////////////////////////////
    // Begin the window
    ImGui::Begin("full size window", nullptr, window_flags);
    try {
        //////////////// initialize ////////////////f
        onImguiStart();

        ////////////////   update   ////////////////
        GameData game_data = updateGameData( );
        loc_ptr->drawLocations(var_ptr, game_data, loc_ptr->visible);
        // send_packets( );

        ////////////////    loop    ////////////////

    }
    catch (const std::string& str) {
        ImGui::End( );
        throw std::string("error on StarCraft UI\n") + str;
    }
    // ImGui::PopStyleColor( ); // Restore previous style

    ////////////
    ImGuiWindowFlags main_window_flags = ImGuiWindowFlags_AlwaysAutoResize ;
    ImGui::Begin("SC:R Debug Window", nullptr, main_window_flags);
    static int currentPage = 0;
    ImGui::SameLine();
    if (ImGui::Button("Inspect", ImVec2(160, 25))) {
        currentPage = 0;
    }
    ImGui::SameLine();
    if (ImGui::Button("Trigger", ImVec2(160, 25))) {
        currentPage = 1;
    }
    ImGui::SameLine();
    if (ImGui::Button("Settings", ImVec2(160, 25))) {
        currentPage = 2;
    }
    ImGui::Separator();

    if (currentPage == 0) {
        if (ImGui::Button("Open EUDVariable inspector")) {
            if (is_var_popup_open) {
                ImGui::SetWindowFocus("EUDVariable Inspector");
            }
            else is_var_popup_open = true;
        }
        else if (ImGui::Button("Open CUnit inspector")) {
            if (is_unit_popup_open) {
                ImGui::SetWindowFocus("CUnit Inspector");
            } else
                is_unit_popup_open = true;
        }
        
    }
    else if (currentPage == 1) {
        /*if (ImGui::Button("sendp")) {
            addPacket(1, 0, 0x11111111, 0x22222222, 0x33333333, 0xFFFFFFFF);
        }*/
    }
    else if (currentPage == 2) {
        ImGuiTreeNodeFlags ImGuiTreeNodeFlags_DefaultOpen = 0;
        if (ImGui::CollapsingHeader("Location Settting", ImGuiTreeNodeFlags_Framed)) {
            if (ImGui::TreeNode("visiblilty")) {
                if (ImGui::Button("Check All")) {
                    for (uint32_t loc_idx = 0; loc_idx < 255; loc_idx++) {
                        if (var_ptr->LocationsUse[loc_idx]) {
                            loc_ptr->visible[loc_idx] = true;
                        }
                    }
                }
                ImGui::SameLine( );
                if (ImGui::Button("UnCheck All")) {
                    for (uint32_t loc_idx = 0; loc_idx < 255; loc_idx++) {
                        if (var_ptr->LocationsUse[loc_idx]) {
                            loc_ptr->visible[loc_idx] = false;
                        }
                    }
                }
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
                ImGui::BeginChild("ChildLocV", ImVec2(528, ImGui::GetTextLineHeight( ) * 38), true,
                                  ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar);
                if (ImGui::BeginTable("table1", 8,
                    ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX)) {
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 85.0f);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 22);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 85.0f);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 22);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 85.0f);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 22);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 85.0f);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 22);
                    uint32_t row_count = 0;
                    ImGui::TableNextRow( );
                    for (uint32_t loc_idx = 0; loc_idx < 255; loc_idx++) {
                        if (var_ptr->LocationsUse[loc_idx]) {
                            ImGui::TableNextColumn( );
                            if (loc_ptr->locations[loc_idx].label == "") ImGui::Text("Location %d", loc_idx + 1);
                            else ImGui::Text(loc_ptr->locations[loc_idx].label.c_str( ));
                            ImGui::TableNextColumn( );
                            ImGui::SetNextItemWidth(16);
                            ImGui::Checkbox(std::format("##{}", loc_idx).c_str( ), &loc_ptr->visible[loc_idx]);
                            row_count++;
                            if (row_count % 4 == 0) {
                                ImGui::TableNextRow( );
                            }
                        }
                    }
                    ImGui::EndTable( );
                }
                ImGui::EndChild( );
                ImGui::PopStyleVar( );
                ImGui::TreePop( );
            }
        }
    }
    // EUDVARIABLE INSPECTOR
    {
        if (is_var_popup_open) {
            ImGui::SetNextWindowSize(ImVec2(545, 600));
        }
        if (is_var_popup_open && ImGui::Begin("EUDVariable Inspector", &is_var_popup_open, ImGuiWindowFlags_NoResize)) {
            static bool isHex = true;

            ImGuiTabBarFlags tab_bar_flags = ImGuiWindowFlags_AlwaysAutoResize;
            if (ImGui::BeginTabBar("EUDVariables tab", tab_bar_flags)) {
                if (ImGui::BeginTabItem("EUDVariables")) {
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
                    ImGui::BeginChild("var_child", ImVec2(540, 480), true, ImGuiWindowFlags_None);

                    int var_idx = 0;
                    int var_table_idx = 0;
                    for (auto& file : var_ptr->file_map) {
                        if (ImGui::CollapsingHeader(std::format("{}", file.first).c_str( ), ImGuiTreeNodeFlags_Framed)) {
                            for (auto& func : file.second) {
                                std::string func_name;
                                if (func.first == "") {
                                    func_name = "globals";
                                } else {
                                    func_name = func.first;
                                }
                                if (ImGui::TreeNode((void*)(intptr_t)var_table_idx, "%s", func_name.c_str( ))) {
                                    ImGuiTableFlags table_flags = ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX;
                                    if (ImGui::BeginTable(std::format("{},##,{}", func_name, var_idx).c_str( ), 5, table_flags)) {
                                        ImGui::TableSetupColumn("type", ImGuiTableColumnFlags_WidthFixed, 92.0f);
                                        ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_WidthFixed, 128.0f);
                                        ImGui::TableSetupColumn("previous", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                                        ImGui::TableSetupColumn("current", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                                        ImGui::TableSetupColumn("Pin", ImGuiTableColumnFlags_WidthFixed, 26.0f);
                                        ImGui::TableHeadersRow( );
                                        for (size_t row = 0; row < func.second.size( ); row++) {
                                            auto& obj = func.second[row].get( );
                                            ImGui::TableNextRow( );
                                            ImGui::TableSetColumnIndex(0);
                                            if (obj.cgfw_type == "") {
                                                ImGui::Text("EUDVariable");

                                                ImGui::TableNextColumn( );
                                                ImGui::Text(obj.var_name.c_str( ));

                                                //// previous value
                                                ImGui::TableNextColumn( );
                                                if (isHex)
                                                    ImGui::Text("0x%08X", obj.prev_value);
                                                else
                                                    ImGui::Text("%d", obj.prev_value);

                                                // current value
                                                ImGui::TableNextColumn( );
                                                inputable_form(isHex, 0, obj, var_idx, writeEUDVariable);

                                                // pinned
                                                ImGui::TableNextColumn( );
                                                ImGui::SetNextItemWidth(25);
                                                if (ImGui::Checkbox(std::format("##pin{}", var_idx).c_str( ), reinterpret_cast<bool*>(&obj.pinned))) {
                                                    std::cout << std::dec << var_idx << ":checked / " << (int)obj.pinned << "\n";
                                                }
                                            } else if (obj.cgfw_type == "EUDArray") {
                                                bool opened = ImGui::TreeNodeEx(std::format("EUDArray##{}", obj.var_name).c_str( ));
                                                ImGui::TableNextColumn( );
                                                ImGui::Text(obj.var_name.c_str( ));
                                                ImGui::TableNextColumn( );
                                                ImGui::TableNextColumn( );
                                                ImGui::Text("size: %d", obj.value);
                                                ImGui::TableNextColumn( );
                                                ImGui::SetNextItemWidth(25);
                                                if (ImGui::Checkbox(std::format("##pin{}", var_idx).c_str( ), reinterpret_cast<bool*>(&obj.pinned))) {
                                                    std::cout << std::dec << var_idx << ":checked / " << (int)obj.pinned << "\n";
                                                }
                                                if (opened) {
                                                    for (size_t arr_idx = 0; arr_idx < obj.value; arr_idx++) {
                                                        ImGui::TableNextRow( );
                                                        ImGui::TableSetColumnIndex(1);
                                                        ImGui::TreeNodeEx(std::format("{}##{}", arr_idx, obj.var_name.c_str( )).c_str( ), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
                                                        ImGui::TableNextColumn( );
                                                        ImGui::TableNextColumn( );
                                                        inputable_form(isHex, arr_idx, obj, var_idx, writeEUDArray);
                                                        var_idx++;
                                                    }
                                                    ImGui::TreePop( );
                                                }
                                            } else if (obj.cgfw_type == "PVariable") {
                                                bool opened = ImGui::TreeNodeEx(std::format("PVariable##{}", obj.var_name).c_str( ));
                                                ImGui::TableNextColumn( );
                                                ImGui::Text(obj.var_name.c_str( ));
                                                ImGui::TableNextColumn( );
                                                ImGui::TableNextColumn( );
                                                ImGui::Text("size: %d", obj.value);
                                                ImGui::TableNextColumn( );
                                                ImGui::SetNextItemWidth(25);
                                                if (ImGui::Checkbox(std::format("##pin{}", var_idx).c_str( ), reinterpret_cast<bool*>(&obj.pinned))) {
                                                    std::cout << std::dec << var_idx << ":checked / " << (int)obj.pinned << "\n";
                                                }
                                                if (opened) {
                                                    for (size_t parr_idx = 0; parr_idx < obj.value; parr_idx++) {
                                                        ImGui::TableNextRow( );
                                                        ImGui::TableSetColumnIndex(1);
                                                        ImGui::TreeNodeEx(std::format("{}##{}", parr_idx, obj.var_name.c_str( )).c_str( ), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
                                                        ImGui::TableNextColumn( );
                                                        ImGui::TableNextColumn( );
                                                        inputable_form(isHex, parr_idx, obj, var_idx, writeEUDVArray);
                                                        var_idx++;
                                                    }
                                                    ImGui::TreePop( );
                                                }
                                            } else if (obj.cgfw_type == "StringBuffer") {
                                                ImGui::Text("StringBuffer");

                                                ImGui::TableNextColumn( );
                                                ImGui::Text(obj.var_name.c_str( ));

                                                ImGui::TableNextColumn( );
                                                if (ImGui::Button(std::format("view##{}{}", obj.var_name, var_idx).c_str( ))) {
                                                    obj.updateDb( );
                                                    obj.mem_edit.Open = true;
                                                    obj.watchingDb = true;
                                                }
                                                ImGui::TableNextColumn( );
                                                ImGui::Text(std::format("size: {}", obj.value).c_str( ));

                                                // pinned
                                                ImGui::TableNextColumn( );
                                                ImGui::SetNextItemWidth(25);
                                                if (ImGui::Checkbox(std::format("##pin{}", var_idx).c_str( ), reinterpret_cast<bool*>(&obj.pinned))) {
                                                    std::cout << std::dec << var_idx << ":checked / " << (int)obj.pinned << "\n";
                                                }
                                            } else if (obj.cgfw_type == "Db") {
                                                ImGui::Text("Db");

                                                ImGui::TableNextColumn( );
                                                ImGui::Text(obj.var_name.c_str( ));

                                                ImGui::TableNextColumn( );
                                                if (ImGui::Button(std::format("view##{}{}", obj.var_name, var_idx).c_str( ))) {
                                                    obj.updateDb( );
                                                    obj.watchingDb = true;
                                                }
                                                ImGui::TableNextColumn( );
                                                ImGui::Text(std::format("size: {}", obj.value).c_str( ));

                                                // pinned
                                                ImGui::TableNextColumn( );
                                                ImGui::SetNextItemWidth(25);
                                                if (ImGui::Checkbox(std::format("##pin{}", var_idx).c_str( ), reinterpret_cast<bool*>(&obj.pinned))) {
                                                    std::cout << std::dec << var_idx << ":checked / " << (int)obj.pinned << "\n";
                                                }
                                            }
                                            var_idx++;
                                        }
                                        ImGui::EndTable( );
                                    }
                                    ImGui::TreePop( );
                                }
                                var_table_idx++;
                            }
                        }
                    }
                    ImGui::EndChild( );
                    ImGui::PopStyleVar( );
                    ImGui::EndTabItem( );
                }
                if (ImGui::BeginTabItem("pin")) {
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
                    ImGui::BeginChild("pin_child", ImVec2(530, 480), true, ImGuiWindowFlags_None);

                    int var_idx = 0;
                    ImGuiTableFlags table_flags = ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX;
                    if (ImGui::BeginTable("pinned", 6, table_flags)) {
                        ImGui::TableSetupColumn("func", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                        ImGui::TableSetupColumn("type", ImGuiTableColumnFlags_WidthFixed, 88.0f);
                        ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_WidthFixed, 88.0f);
                        ImGui::TableSetupColumn("previous", ImGuiTableColumnFlags_WidthFixed, 86.0f);
                        ImGui::TableSetupColumn("current", ImGuiTableColumnFlags_WidthFixed, 86.0f);
                        ImGui::TableSetupColumn("Pin", ImGuiTableColumnFlags_WidthFixed, 26.0f);
                        ImGui::TableHeadersRow( );
                        for (auto& file : var_ptr->file_map) {
                            for (auto& func : file.second) {
                                for (size_t row = 0; row < func.second.size( ); row++) {
                                    auto& obj = func.second[row].get( );
                                    if (!obj.pinned)
                                        continue;
                                    ImGui::TableNextRow( );
                                    ImGui::Text(obj.func_name.c_str( ));
                                    ImGui::TableSetColumnIndex(1);
                                    if (obj.cgfw_type == "") {
                                        ImGui::Text("EUDVariable");

                                        ImGui::TableNextColumn( );
                                        ImGui::Text(obj.var_name.c_str( ));

                                        //// previous value
                                        ImGui::TableNextColumn( );
                                        if (isHex)
                                            ImGui::Text("0x%08X", obj.prev_value);
                                        else
                                            ImGui::Text("%d", obj.prev_value);

                                        // current value
                                        ImGui::TableNextColumn( );
                                        inputable_form(isHex, 0, obj, var_idx, writeEUDVariable);

                                        // pinned
                                        ImGui::TableNextColumn( );
                                        ImGui::SetNextItemWidth(25);
                                        if (ImGui::Checkbox(std::format("##pin{}", var_idx).c_str( ), reinterpret_cast<bool*>(&obj.pinned))) {
                                            std::cout << std::dec << var_idx << ":checked / " << (int)obj.pinned << "\n";
                                        }
                                    } else if (obj.cgfw_type == "EUDArray") {
                                        bool opened = ImGui::TreeNodeEx(std::format("EUDArray##{}", obj.var_name).c_str( ));
                                        ImGui::TableNextColumn( );
                                        ImGui::Text(obj.var_name.c_str( ));
                                        ImGui::TableNextColumn( );
                                        ImGui::TableNextColumn( );
                                        ImGui::Text("size: %d", obj.value);
                                        ImGui::TableNextColumn( );
                                        ImGui::SetNextItemWidth(25);
                                        if (ImGui::Checkbox(std::format("##pin{}", var_idx).c_str( ), reinterpret_cast<bool*>(&obj.pinned))) {
                                            std::cout << std::dec << var_idx << ":checked / " << (int)obj.pinned << "\n";
                                        }
                                        if (opened) {
                                            for (size_t arr_idx = 0; arr_idx < obj.value; arr_idx++) {
                                                ImGui::TableNextRow( );
                                                ImGui::TableSetColumnIndex(2);
                                                ImGui::TreeNodeEx(std::format("{}##{}", arr_idx, obj.var_name.c_str( )).c_str( ), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
                                                ImGui::TableNextColumn( );
                                                ImGui::TableNextColumn( );
                                                inputable_form(isHex, arr_idx, obj, var_idx, writeEUDArray);
                                                var_idx++;
                                            }
                                            ImGui::TreePop( );
                                        }
                                    } else if (obj.cgfw_type == "PVariable") {
                                        bool opened = ImGui::TreeNodeEx(std::format("PVariable##{}", obj.var_name).c_str( ));
                                        ImGui::TableNextColumn( );
                                        ImGui::Text(obj.var_name.c_str( ));
                                        ImGui::TableNextColumn( );
                                        ImGui::TableNextColumn( );
                                        ImGui::Text("size: %d", obj.value);
                                        ImGui::TableNextColumn( );
                                        ImGui::SetNextItemWidth(25);
                                        if (ImGui::Checkbox(std::format("##pin{}", var_idx).c_str( ), reinterpret_cast<bool*>(&obj.pinned))) {
                                            std::cout << std::dec << var_idx << ":checked / " << (int)obj.pinned << "\n";
                                        }
                                        if (opened) {
                                            for (size_t parr_idx = 0; parr_idx < obj.value; parr_idx++) {
                                                ImGui::TableNextRow( );
                                                ImGui::TableSetColumnIndex(2);
                                                ImGui::TreeNodeEx(std::format("{}##{}", parr_idx, obj.var_name.c_str( )).c_str( ), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
                                                ImGui::TableNextColumn( );
                                                ImGui::TableNextColumn( );
                                                inputable_form(isHex, parr_idx, obj, var_idx, writeEUDVArray);
                                                var_idx++;
                                            }
                                            ImGui::TreePop( );
                                        }
                                    } else if (obj.cgfw_type == "StringBuffer") {
                                        ImGui::Text("StringBuffer");

                                        ImGui::TableNextColumn( );
                                        ImGui::Text(obj.var_name.c_str( ));

                                        ImGui::TableNextColumn( );
                                        if (ImGui::Button(std::format("view##{}{}", obj.var_name, var_idx).c_str( ))) {
                                            obj.updateDb( );
                                            obj.mem_edit.Open = true;
                                            obj.watchingDb = true;
                                        }
                                        ImGui::TableNextColumn( );
                                        ImGui::Text(std::format("size: {}", obj.value).c_str( ));

                                        // pinned
                                        ImGui::TableNextColumn( );
                                        ImGui::SetNextItemWidth(25);
                                        if (ImGui::Checkbox(std::format("##pin{}", var_idx).c_str( ), reinterpret_cast<bool*>(&obj.pinned))) {
                                            std::cout << std::dec << var_idx << ":checked / " << (int)obj.pinned << "\n";
                                        }
                                    } else if (obj.cgfw_type == "Db") {
                                        ImGui::Text("Db");

                                        ImGui::TableNextColumn( );
                                        ImGui::Text(obj.var_name.c_str( ));

                                        ImGui::TableNextColumn( );
                                        if (ImGui::Button(std::format("view##{}{}", obj.var_name, var_idx).c_str( ))) {
                                            obj.updateDb( );
                                            obj.watchingDb = true;
                                        }
                                        ImGui::TableNextColumn( );
                                        ImGui::Text(std::format("size: {}", obj.value).c_str( ));

                                        // pinned
                                        ImGui::TableNextColumn( );
                                        ImGui::SetNextItemWidth(25);
                                        if (ImGui::Checkbox(std::format("##pin{}", var_idx).c_str( ), reinterpret_cast<bool*>(&obj.pinned))) {
                                            std::cout << std::dec << var_idx << ":checked / " << (int)obj.pinned << "\n";
                                        }
                                    }
                                    var_idx++;
                                }
                            }
                        }
                        ImGui::EndTable();
                    }
                    ImGui::EndChild();
                    ImGui::PopStyleVar();
                    ImGui::EndTabItem();
                }
            }
            // things same line with tabs
            float windowWidth = ImGui::GetWindowWidth();
            float buttonWidth = 180.0f;
            ImGui::SetCursorPosX(windowWidth - buttonWidth);
            ImGui::SameLine();
            ImGui::Text("                                      ");
            ImGui::SameLine();
            ImGui::Text("Dec");
            ImGui::SameLine();
            ToggleButton("hexdec", &isHex);
            ImGui::SameLine();
            ImGui::Text("Hex");
            ImGui::SameLine();
            ImGui::Separator();
            ImGui::EndTabBar();
            ImGui::End();
        }
    }

    // CUNIT INSPECTOR
    {
        static bool isHex = true;
        if (is_unit_popup_open) {
            ImGui::SetNextWindowSize(ImVec2(500, 600));
        }
        if (is_unit_popup_open && ImGui::Begin("CUnit Inspector", &is_unit_popup_open, ImGuiWindowFlags_NoResize)) {
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
            float windowWidth = ImGui::GetWindowWidth( );
            float buttonWidth = 100.0f;
            ImGui::SetCursorPosX(windowWidth - buttonWidth);
            ImGui::Text("Dec");
            ImGui::SameLine( );
            ToggleButton("hexdec", &isHex);
            ImGui::SameLine( );
            ImGui::Text("Hex");
            ImGui::SameLine( );
            ImGui::Separator( );
            ImGui::BeginChild("var_child", ImVec2(485, 480), true, ImGuiWindowFlags_None);
            ImGui::Text("* Selected Unit");
            ImGuiTableFlags table_flags = ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX;
            if (ImGui::BeginTable("selected cunit table", 5, table_flags)) {
                ImGui::TableSetupColumn("index", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableSetupColumn("player", ImGuiTableColumnFlags_WidthFixed, 30.0f);
                ImGui::TableSetupColumn("hp ", ImGuiTableColumnFlags_WidthFixed, 78.0f);
                ImGui::TableSetupColumn("unittype", ImGuiTableColumnFlags_WidthFixed, 190.0f);
                ImGui::TableSetupColumn("button", ImGuiTableColumnFlags_WidthFixed, 38.0f);
                for (auto& idx: var_ptr->selectedUnit) {
                    if (idx == 0xFFFFFFFF) { continue; }
                    if (unit_ptr->cunits[idx].orderID != 0) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text(std::format("index: {}", idx).c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text(std::format("P{}", unit_ptr->cunits[idx].playerID + 1).c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text(std::format("hp:{}", unit_ptr->cunits[idx].hp >> 8).c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text(std::format("{}", UnitDict[unit_ptr->cunits[idx].unitID]).c_str());
                        ImGui::TableNextColumn();
                        if (ImGui::Button(std::format("View##idx{}", idx).c_str())) {
                            unit_ptr->isDisplaying[idx] = true;
                        }
                    }
                }
                ImGui::EndTable();
            }
            ImGui::Separator();
            ImGui::Text("* All Existing Unit");
            if (ImGui::BeginTable("cunit table", 5, table_flags)) {
                ImGui::TableSetupColumn("index", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableSetupColumn("player", ImGuiTableColumnFlags_WidthFixed, 30.0f);
                ImGui::TableSetupColumn("hp ", ImGuiTableColumnFlags_WidthFixed, 78.0f);
                ImGui::TableSetupColumn("unittype", ImGuiTableColumnFlags_WidthFixed, 190.0f);
                ImGui::TableSetupColumn("button", ImGuiTableColumnFlags_WidthFixed, 38.0f);
                for (size_t idx = 0; idx < 1700; idx++) {
                    if (unit_ptr->cunits[idx].orderID != 0) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text(std::format("index: {}", idx).c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text(std::format("P{}", unit_ptr->cunits[idx].playerID + 1).c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text(std::format("hp:{}", unit_ptr->cunits[idx].hp >> 8).c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text(std::format("{}", UnitDict[unit_ptr->cunits[idx].unitID]).c_str());
                        ImGui::TableNextColumn();
                        if (ImGui::Button(std::format("View##idx{}", idx).c_str())) {
                            unit_ptr->isDisplaying[idx] = true;
                        }
                    }
                }
                ImGui::EndTable();
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::End();
        }
        for (size_t idx = 0; idx < 1700; idx++) {
            if (!unit_ptr->isDisplaying[idx]) continue;
            ImGui::SetNextWindowSize(ImVec2(445, 700), ImGuiCond_FirstUseEver);
            if (unit_ptr->isDisplaying[idx] && ImGui::Begin(std::format("CUnit viewer / index:{}", idx).c_str( ), &unit_ptr->isDisplaying[idx])) {
                ImGui::Text(std::format("CUnit Layout of [HP: {} / {} / {}]", unit_ptr->cunits[idx].hp >> 8, UnitDict[unit_ptr->cunits[idx].unitID], unit_ptr->cunits[idx].orderID == 0 ? "Dead" : "Alive").c_str());
                ImGui::Separator();
                ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.98f, ImGui::GetContentRegionAvail().y * 0.99f), ImGuiWindowFlags_HorizontalScrollbar);
                ImGuiTableFlags table_flags = ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX;
                if (ImGui::BeginTable("CUnitTable", 4, table_flags)) {
                    ImGui::TableSetupColumn("Offset", ImGuiTableColumnFlags_WidthFixed, 42.0f);
                    ImGui::TableSetupColumn("Field Name", ImGuiTableColumnFlags_WidthFixed, 190.0f);
                    ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 38.0f);
                    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 85.0f);
                    ImGui::TableHeadersRow();

                    int offset = 0;
                    for (const auto& field : CUnitFields) {
                        std::string fieldName = field.first;
                        int fieldSize = field.second;
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("0x%03X", offset);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%s", fieldName.c_str());
                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("%d", fieldSize);
                        ImGui::TableSetColumnIndex(3);

                        // handle cunit and csprite
                        if (fieldSize == 4 && (offset == offsetof(CUnit, prev) || offset == offsetof(CUnit, next) ||
                            offset == offsetof(CUnit, moveTarget) || offset == offsetof(CUnit, orderTarget) ||
                            offset == offsetof(CUnit, prevPlayerUnit) || offset == offsetof(CUnit, nextPlayerUnit) ||
                            offset == offsetof(CUnit, subUnit) || offset == offsetof(CUnit, autoTargetUnit) ||
                            offset == offsetof(CUnit, connectedUnit) || offset == offsetof(CUnit, addon))) {
                            if (ImGui::Button(std::format("CUnit View##{}/{}", idx, offset).c_str())) {
                                uint32_t value = *(uint32_t*)((char*)&unit_ptr->cunits[idx] + offset);
                                uint32_t cunit_base = getUnittableAddr( ) + 8;
                                
                                if (value < cunit_base || value >= (cunit_base + 1700 * 336)) continue;
                                uint32_t sel_index = (value - cunit_base) / 336;
                                unit_ptr->isDisplaying[sel_index] = true;
                            }

                            if (ImGui::IsItemHovered( )) {
                                uint32_t value = *(uint32_t*)((char*)&unit_ptr->cunits[idx] + offset);
                                uint32_t cunit_base = getUnittableAddr( ) + 8;
                                if (value < cunit_base || value >= (cunit_base + 1700 * 336)) {
                                    ImGui::SetTooltip(std::format("0x{:08X}", value).c_str( ));
                                } else {
                                    ImGui::SetTooltip(std::format("index: {}", (value - cunit_base) / 336).c_str( ));
                                }
                            }
                            offset += 4;
                            continue;
                        }
                        else if (offset == offsetof(CUnit, sprite)) {
                            // todo: add csprite inspector
                        }

                        if (isHex) {
                            char dummy[3] = "0x";
                            ImGui::SetNextItemWidth(20);
                            ImGui::InputText(std::format("##cunit_hex_prefix{}", idx).c_str(), dummy, 2, ImGuiInputTextFlags_ReadOnly);
                            ImGui::SameLine(0.0f, 0.0f);
                        }
                        if (fieldSize == 1) {
                            char buffer[20];
                            if (isHex) {
                                uint8_t value = *(uint8_t*)((char*)&unit_ptr->cunits[idx] + offset);
                                std::snprintf(buffer, sizeof(buffer), "%02x", value);
                                ImGui::SetNextItemWidth(20);
                                ImGui::InputText(std::format("##cunit_form_hex{}/{}", idx, offset).c_str(), buffer, 3, ImGuiInputTextFlags_CharsHexadecimal);
                                if (ImGui::IsItemDeactivatedAfterEdit()) {
                                    uint8_t ret = std::stoul(buffer, nullptr, 16);
                                    if (ret >= 0x100)
                                        ret = 0xFF;
                                    Internal::bwrite((uint32_t)(getUnittableAddr() + 8 + idx * 336 + offset), ret);
                                }
                            }
                            else {
                                uint8_t value = *(uint8_t*)((char*)&unit_ptr->cunits[idx] + offset);
                                std::snprintf(buffer, sizeof(buffer), "%u", value);
                                ImGui::SetNextItemWidth(40);
                                ImGui::InputText(std::format("##cunit_form_hex{}/{}", idx, offset).c_str(), buffer, 4, ImGuiInputTextFlags_CharsDecimal);
                                if (ImGui::IsItemDeactivatedAfterEdit()) {
                                    uint8_t ret = std::stoul(buffer, nullptr, 10);
                                    if (ret >= 0x100)
                                        ret = 0xFF;
                                    Internal::bwrite((uint32_t)(getUnittableAddr() + 8 + idx * 336 + offset), ret);
                                }
                            }
                        }
                        else if (fieldSize == 2) {
                            char buffer[20];
                            if (isHex) {
                                uint16_t value = *(uint16_t*)((char*)&unit_ptr->cunits[idx] + offset);
                                std::snprintf(buffer, sizeof(buffer), "%04x", value);
                                ImGui::SetNextItemWidth(40);
                                ImGui::InputText(std::format("##cunit_form_hex{}/{}", idx, offset).c_str(), buffer, 5, ImGuiInputTextFlags_CharsHexadecimal);
                                if (ImGui::IsItemDeactivatedAfterEdit()) {
                                    uint16_t ret = std::stoul(buffer, nullptr, 16);
                                    if (ret >= 0x10000)
                                        ret = 0xFFFF;
                                    Internal::wwrite((uint32_t)(getUnittableAddr() + 8 + idx * 336 + offset), ret);
                                }
                            }
                            else {
                                uint16_t value = *(uint16_t*)((char*)&unit_ptr->cunits[idx] + offset);
                                std::snprintf(buffer, sizeof(buffer), "%u", value);
                                ImGui::SetNextItemWidth(60);
                                ImGui::InputText(std::format("##cunit_form_hex{}/{}", idx, offset).c_str(), buffer, 6, ImGuiInputTextFlags_CharsDecimal);
                                if (ImGui::IsItemDeactivatedAfterEdit()) {
                                    uint16_t ret = std::stoul(buffer, nullptr, 10);
                                    if (ret >= 0x10000)
                                        ret = 0xFFFF;
                                    Internal::wwrite((uint32_t)(getUnittableAddr() + 8 + idx * 336 + offset), ret);
                                }
                            }
                        }
                        else if (fieldSize == 4) {
                            char buffer[20];
                            if (isHex) {
                                uint32_t value = *(uint32_t*)((char*)&unit_ptr->cunits[idx] + offset);
                                std::snprintf(buffer, sizeof(buffer), "%08x", value);
                                ImGui::SetNextItemWidth(80);
                                ImGui::InputText(std::format("##cunit_form_hex{}/{}", idx, offset).c_str( ), buffer, 9, ImGuiInputTextFlags_CharsHexadecimal);
                                if (ImGui::IsItemDeactivatedAfterEdit( )) {
                                    uint64_t ret = std::stoull(buffer, nullptr, 16);
                                    if (ret >= 0xFFFFFFFF)
                                        ret = 0xFFFFFFFF;
                                    Internal::dwwrite((uint32_t)(getUnittableAddr( ) + 8 + idx * 336 + offset), static_cast<uint32_t>(ret));
                                }
                            } else {
                                uint32_t value = *(uint32_t*)((char*)&unit_ptr->cunits[idx] + offset);
                                std::snprintf(buffer, sizeof(buffer), "%u", value);
                                ImGui::SetNextItemWidth(125);
                                ImGui::InputText(std::format("##cunit_form_hex{}/{}", idx, offset).c_str( ), buffer, 11, ImGuiInputTextFlags_CharsDecimal);
                                if (ImGui::IsItemDeactivatedAfterEdit( )) {
                                    uint64_t ret = std::stoull(buffer, nullptr, 10);
                                    if (ret >= 0xFFFFFFFF)
                                        ret = 0xFFFFFFFF;
                                    Internal::dwwrite((uint32_t)(getUnittableAddr( ) + 8 + idx * 336 + offset), static_cast<uint32_t>(ret));
                                }
                            }
                        }
                        offset += fieldSize;
                    }
                }
                ImGui::EndTable( );
                ImGui::EndChild( );
                ImGui::End( );
            }
        }
    }
    // End the window
    ImGui::End( );
    ///////////////////////////////////////////////////////////////////////////////
}
