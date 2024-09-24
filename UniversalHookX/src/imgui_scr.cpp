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

// Global variables
ID3D11Device* g_pd3dDevice = NULL;
ID3D11DeviceContext* g_pd3dDeviceContext = NULL;

std::unique_ptr<Variables> var_ptr;
std::unique_ptr<Locations> loc_ptr;
bool starcraft_input = true;
static bool is_var_popup_open = false;
static bool isLocationVisible[255];
GameData updateGameData( ) {
    // from eudplib
    var_ptr->update_value( );

    // from process
    uint32_t exeAddr = getEXEAddr( );
    GameData gdata;
    gdata.screen_size_x  = Internal::dwread(exeAddr + 0xB31AC0);
    gdata.screen_size_y  = Internal::dwread(exeAddr + 0xB31AC8);
    gdata.console_height = Internal::dwread(exeAddr + 0xB31AC4); // unsued
    gdata.pillar_size    = Internal::dwread(exeAddr + 0xDE6104);
    loc_ptr->updateData( );
    return gdata;
}


void onImguiStart() {
    if (var_ptr) return;
    // from eudplib
    var_ptr = std::make_unique<Variables>(getVariables( ));

    // from process
    if (OpenTargetProcess( )) {
        setEXEAddr(GetModuleBaseAddress(L"StarCraft.exe"));
        setUnittableAddr(GetModuleBaseAddress(L"Opengl32.dll"));
        std::cout << "unittable: 0x" << std::hex << getUnittableAddr( ) << "\n";
        if (getUnittableAddr( ) == 0x60000) {
            setUnittableAddr(findUnitableAddr());
            std::cout << "unittable: 0x" << std::hex << getUnittableAddr( ) << "\n";
        }
    }
    loc_ptr = std::make_unique<Locations>(findMRGNAddr(), var_ptr->Locations);
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
    ImGui::Begin("My Full-Sized Window", nullptr, window_flags);
    ImGui::PushFont(getFont(30)); // Use the specific font size
    ImGui::Text("This text uses the Kostart10 font.");
    //////////////// initialize ////////////////
    onImguiStart();

    ////////////////   update   ////////////////
    var_ptr->update_value( );
    GameData game_data = updateGameData( );
    loc_ptr->drawLocations(var_ptr, game_data);

    ////////////////    loop    ////////////////


    // Set the background color to fully transparent
        // ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0)); // Fully transparent
    ImGui::Text("Hello, World!");                                 // You can add other UI elements here
    if (ImGui::Button("Click Me")) {
        // Button action
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
        
    }
    else if (currentPage == 2) {
        ImGuiTreeNodeFlags ImGuiTreeNodeFlags_DefaultOpen = 0;
        if (ImGui::CollapsingHeader("Location Settting", ImGuiTreeNodeFlags_Framed)) {
            if (ImGui::TreeNode("visiblilty")) {
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
                    for (int row = 0; row < 64; row++) {
                            ImGui::TableNextRow( );
                        for (int column = 0; column < 8; column+=2) {
                            if (int(row * 4 + column / 2) >= 255) break;
                            ImGui::TableSetColumnIndex(column);
                            if (loc_ptr->locations[row * 4 + column / 2].label == "") {
                                ImGui::Text("Location %d", row * 4 + column / 2 + 1);
                            }
                            else {
                                ImGui::Text(loc_ptr->locations[row * 4 + column / 2].label.c_str( ));
                            }
   
                        }
                        for (int column = 1; column < 8; column += 2) {
                            if (int(row * 4 + column / 2) >= 255) break;
                            ImGui::TableSetColumnIndex(column);
                            ImGui::SetNextItemWidth(16);
                            ImGui::Checkbox("", &isLocationVisible[row * 4 + column / 2]);
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

    if (is_var_popup_open) {
        ImGui::SetNextWindowSize(ImVec2(545, 600));
    }
    if (is_var_popup_open && ImGui::Begin("EUDVariable Inspector", nullptr, ImGuiWindowFlags_NoResize)) {
        static bool isHex = true;
        // things same line with tabs
        float windowWidth = ImGui::GetWindowWidth( );
        float buttonWidth = 150.0f; // 원하는 버튼 너비
        ImGui::SetCursorPosX(windowWidth - buttonWidth);
        ImGui::Text("Dec");
        ImGui::SameLine( );
        ToggleButton("hexdec", &isHex);
        ImGui::SameLine( );
        ImGui::Text("Hex");
        ImGui::SameLine( );
        ImGui::Text("   ");
        ImGui::SameLine( );
        if (ImGui::Button("Close"))
            is_var_popup_open = false;
        // ImGui::Separator( );

        ImGuiTabBarFlags tab_bar_flags = ImGuiWindowFlags_AlwaysAutoResize;
        if (ImGui::BeginTabBar("EUDVariables tab", tab_bar_flags)) {
            if (ImGui::BeginTabItem("EUDVariables")) {
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
                ImGui::BeginChild("var_child", ImVec2(530, 480), true, ImGuiWindowFlags_None);

                int var_idx = 0;
                int var_table_idx = 0;
                for (auto& file : var_ptr->file_map) {
                    if (ImGui::CollapsingHeader(std::format("{}", file.first).c_str( ), ImGuiTreeNodeFlags_Framed)) {
                        for (auto& func : file.second) {
                            std::string func_name;
                            if (func.first == "") { func_name = "globals"; }
                            else { func_name = func.first; }
                            if (ImGui::TreeNode((void*)(intptr_t)var_table_idx, "%s", func_name.c_str( ))) {
                                ImGuiTableFlags table_flags = ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX;

                                if (ImGui::BeginTable(std::format("{},##,{}", func_name, var_idx).c_str(), 5, table_flags)) {
                                    ImGui::TableSetupColumn("type", ImGuiTableColumnFlags_WidthFixed, 90.0f);
                                    ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_WidthFixed, 120.0f);
                                    ImGui::TableSetupColumn("previous", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                                    ImGui::TableSetupColumn("current", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                                    ImGui::TableSetupColumn("Pin", ImGuiTableColumnFlags_WidthFixed, 26.0f);
                                    ImGui::TableHeadersRow( );
                                    for (int row = 0; row < func.second.size( ); row++) {
                                        auto& obj = func.second[row];
                                        ImGui::TableNextRow( );
                                        ImGui::TableSetColumnIndex(0);
                                        if (obj.get( ).cgfw_type == "") {
                                            ImGui::Text("EUDVariable");

                                            ImGui::TableNextColumn();
                                            ImGui::Text(obj.get( ).var_name.c_str( ));

                                            // previous value
                                            ImGui::TableNextColumn();
                                            if (isHex)
                                                ImGui::Text("0x%08X", obj.get( ).prev_value);
                                            else
                                                ImGui::Text("%d", obj.get( ).prev_value);

                                            // current value
                                            ImGui::TableNextColumn();
                                            inputable_form(isHex, 0, obj, var_idx, writeEUDVariable);

                                            // pinned
                                            ImGui::TableNextColumn();
                                            ImGui::SetNextItemWidth(25);
                                            if (ImGui::Checkbox(std::format("##pin{}", var_idx).c_str( ), reinterpret_cast<bool*>(&obj.get( ).pinned))) {
                                                std::cout << std::dec << var_idx / 2 << ":checked / " << (int)obj.get( ).pinned << "\n";
                                            }
                                            var_idx += 2;
                                        } 
                                        else if(obj.get( ).cgfw_type == "EUDArray") {
                                            bool opened = ImGui::TreeNodeEx("EUDArray");
                                            ImGui::TableNextColumn( );
                                            ImGui::Text(obj.get( ).var_name.c_str( ));
                                            ImGui::TableNextColumn( );
                                            ImGui::TableNextColumn( );
                                            ImGui::Text("size: %d", obj.get( ).value);
                                            if (opened) {
                                                for (size_t arr_idx = 0; arr_idx < obj.get().value; arr_idx++) {
                                                    ImGui::TableNextRow( );
                                                    ImGui::TableSetColumnIndex(1);
                                                    ImGui::TreeNodeEx(std::format("{}##{}", arr_idx, obj.get( ).var_name.c_str( )).c_str( ), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
                                                    ImGui::TableNextColumn( );
                                                    ImGui::TableNextColumn( );
                                                    inputable_form(isHex, arr_idx, obj, var_idx, writeEUDArray);
                                                }
                                                ImGui::TreePop( );
                                            }
                                        }
                                        else if (obj.get( ).cgfw_type == "PVariable") {
                                            bool opened = ImGui::TreeNodeEx("PVariable");
                                            ImGui::TableNextColumn( );
                                            ImGui::Text(obj.get( ).var_name.c_str( ));
                                            ImGui::TableNextColumn( );
                                            ImGui::TableNextColumn( );
                                            ImGui::Text("size: %d", obj.get( ).value);
                                            if (opened) {
                                                for (size_t parr_idx = 0; parr_idx < obj.get( ).value; parr_idx++) {
                                                    ImGui::TableNextRow( );
                                                    ImGui::TableSetColumnIndex(1);
                                                    ImGui::TreeNodeEx(std::format("{}##{}", parr_idx, obj.get( ).var_name.c_str( )).c_str( ), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
                                                    ImGui::TableNextColumn( );
                                                    ImGui::TableNextColumn( );
                                                    inputable_form(isHex, parr_idx, obj, var_idx, writeEUDVArray);
                                                }
                                                ImGui::TreePop( );
                                            }
                                        }
                                    }
                                    ImGui::EndTable( );
                                }
                                ImGui::TreePop( );
                            }
                            else { var_idx += (2 * file.second.size( )); }
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
                ImGui::BeginChild("pin_child", ImVec2(470, 480), true, ImGuiWindowFlags_None);

                int var_idx = 0;
                int var_table_idx = 0;

                /*
                for (auto& func_var : var_ptr->func_var) {
                    bool pin_flag = false;
                    for (size_t i = 0; i < func_var.second.size( ); i++) {
                        if (var_pin[var_idx / 2 + i] == true) {
                            pin_flag = true;
                            break;
                        }
                    }
                    if (!pin_flag) {
                        var_idx += (2 * func_var.second.size( ));
                        continue;
                    }
                    if (ImGui::TreeNode((void*)(intptr_t)var_table_idx, "%s", func_var.first.c_str( ))) {
                        ImGuiTableFlags table_flags = ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX;
                        if (ImGui::BeginTable(func_var.first.c_str( ), 4, table_flags)) {
                            ImGui::TableSetupColumn("EUDVariable", ImGuiTableColumnFlags_WidthFixed, 130.0f);
                            ImGui::TableSetupColumn("previous", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                            ImGui::TableSetupColumn("current", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                            ImGui::TableSetupColumn("Pin", ImGuiTableColumnFlags_WidthFixed, 26.0f);
                            ImGui::TableHeadersRow( );
                            
                            for (int row = 0; row < func_var.second.size( ); row++) {
                                if (!var_pin[var_idx / 2]) {
                                    var_idx += 2;
                                    continue;
                                }
                                ImGui::TableNextRow( );
                                ImGui::TableSetColumnIndex(0);
                                ImGui::Text(var_ptr->strtable.var_str[func_var.second[row].get( ).var_index].c_str( ));

                                // previous value
                                ImGui::TableSetColumnIndex(1);
                                if (isHex)
                                    ImGui::Text("0x%08X", var_buf[var_idx]);
                                else
                                    ImGui::Text("%d", var_buf[var_idx]);

                                // current value
                                ImGui::TableSetColumnIndex(2);
                                var_buf[var_idx + 1] = func_var.second[row].get( ).value;
                                if (isHex) {
                                    char buf[11];
                                    snprintf(buf, sizeof(buf), "0x%08X", var_buf[var_idx + 1]); // 16진수로 변환
                                    ImGui::SetNextItemWidth(90);
                                    ImGui::InputText(std::format("##var_hex{}", var_idx).c_str( ), buf, sizeof(buf), ImGuiInputTextFlags_CharsHexadecimal);
                                    if (ImGui::IsItemDeactivatedAfterEdit( )) {
                                        std::cout << "foucus\n";
                                        std::cout << "buf: " << std::hex << buf << " / " << static_cast<uint32_t>(std::stoul(buf + 2, nullptr, 16)) << "\n";
                                        dwwrite(func_var.second[row].get( ).address, static_cast<uint32_t>(std::stoul(buf + 2, nullptr, 16)));
                                    }
                                } else {
                                    char buf[20];
                                    snprintf(buf, sizeof(buf), "%d", var_buf[var_idx + 1]); // 10진수로 변환
                                    ImGui::SetNextItemWidth(90);
                                    ImGui::InputText(std::format("##var_dec{}", var_idx).c_str( ), buf, sizeof(buf), ImGuiInputTextFlags_CharsDecimal);
                                    if (ImGui::IsItemDeactivatedAfterEdit( )) {
                                        std::cout << "foucus\n";
                                        dwwrite(func_var.second[row].get( ).address, static_cast<uint32_t>(std::stoul(buf, nullptr, 10)));
                                    }
                                }

                                // pinned
                                ImGui::TableSetColumnIndex(3);
                                ImGui::SetNextItemWidth(25);
                                if (ImGui::Checkbox(std::format("##pin{}", var_idx).c_str( ), reinterpret_cast<bool*>(&var_pin[var_idx / 2]))) {
                                    std::cout << std::dec << var_idx / 2 << ":checked / " << (int)var_pin[var_idx / 2] << "\n";
                                }
                                var_idx += 2;
                            }
                            ImGui::EndTable( );
                        }
                        ImGui::TreePop( );
                    }
                    else var_idx += (2 * func_var.second.size( ));
                    var_table_idx++;
                }
                ImGui::EndChild( );
                ImGui::PopStyleVar( );
                ImGui::EndTabItem( );
                */
            }
        }
        ImGui::EndTabBar( );
        ImGui::End( );
        
    }
    ImGui::PopFont( ); // Revert to previous font

    //  end_signature( );
    // End the window
    ImGui::End( );
    ///////////////////////////////////////////////////////////////////////////////
}

void Debug_UI() {

}
