#include "imgui_scr.h"
#include "memoryRW.h"
#include "debugger_main.h"
#include "signature.h"
#include "console/console.hpp"
#include <memory>
#include <codecvt>
#include <locale>

// Global variables
ID3D11Device* g_pd3dDevice = NULL;
ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
std::unique_ptr<Variables> var_ptr;
std::unique_ptr<Locations> loc_ptr;

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
    loc_ptr->updateData();
    return gdata;
}


void onImguiStart() {
    if (var_ptr) return;
    LOG("=========== var_ptr: 0x%08X =============\n", var_ptr.get());
    // from eudplib
    var_ptr = std::make_unique<Variables>(getVariables( ));

    // from process
    // initFonts( );
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
bool starcraft_input = true;
static bool is_var_popup_open = false;
static bool isLocationVisible[255];
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
    if (ImGui::Button("Inspect", ImVec2(150, 25))) {
        currentPage = 0;
    }
    ImGui::SameLine();
    if (ImGui::Button("Trigger", ImVec2(150, 25))) {
        currentPage = 1;
    }
    ImGui::SameLine();
    if (ImGui::Button("Settings", ImVec2(150, 25))) {
        currentPage = 2;
    }
    ImGui::Separator();

    if (currentPage == 0) {
        if (ImGui::Button("Open EUDVariable inspector")) {
            is_var_popup_open = true;
        }
        
    }
    else if (currentPage == 2) {
        ImGuiTreeNodeFlags ImGuiTreeNodeFlags_DefaultOpen = 0;
        if (ImGui::CollapsingHeader("Location Settting", ImGuiTreeNodeFlags_Framed)) {
            if (ImGui::TreeNode("visiblilty")) {
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
                ImGui::BeginChild("ChildLocV", ImVec2(500, ImGui::GetTextLineHeight( ) * 30), true, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar);
                if (ImGui::BeginTable("table1", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoHostExtendX)) {
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 20);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 20);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 20);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 20);
                    for (int row = 0; row < 64; row++) {
                        ImGui::TableNextRow( );
                        ImGui::TableSetColumnIndex(0);
                        for (int column = 0; column < 8; column+=2) {
                            if (row * 4 + column / 2 > 255)
                                break;
                                ImGui::TableSetColumnIndex(column);
                            // ImGui::SetNextItemWidth(80);
                            if (loc_ptr->locations[row * 4 + column / 2].label == "") {
                                ImGui::Text("Location %d", row * 4 + column / 2 + 1);
                            } else {
                                ImGui::Text(loc_ptr->locations[row * 4 + column / 2].label.c_str( ));
                            }
                        }
                        for (int column = 1; column < 8; column += 2) {
                            if (row * 4 + column / 2 > 255)
                                break;
                            ImGui::TableSetColumnIndex(column);
                            ImGui::SetNextItemWidth(16);
                            ImGui::Checkbox("", &isLocationVisible[row * 4 + column / 2]);
                        }
                    }
                    ImGui::EndTable( );
                    ImGui::EndChild( );
                    ImGui::PopStyleVar( );
                    ImGui::EndTabItem( );
                }
            }
        }
    }

    if (is_var_popup_open)
        ImGui::SetNextWindowSize(ImVec2(500, 680));
    if (is_var_popup_open && ImGui::Begin("EUDVariable Popup", nullptr, ImGuiWindowFlags_None)) {
        static bool isHex = true;
            ImGui::BeginChild("tab_child", ImVec2(480, 580), true, window_flags);
            ImGui::Text("This is a popup.");
            if (ImGui::Button("Close")) { is_var_popup_open = false; }

            ImGuiTabBarFlags tab_bar_flags = ImGuiWindowFlags_AlwaysAutoResize;
            if (ImGui::BeginTabBar("EUDVariables tab", tab_bar_flags)) {
                if (ImGui::BeginTabItem("EUDVariables")) {
                    ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
                    window_flags |= ImGuiWindowFlags_NoScrollWithMouse;
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
                    ImGui::BeginChild("var_child", ImVec2(480, 580), true, window_flags);

                    int var_table_idx = 0;
                    for (auto& func_var : var_ptr->func_var) {
                        if (ImGui::TreeNode((void*)(intptr_t)var_table_idx, "%s", func_var.first.c_str( ))) {
                            ImGuiTableFlags table_flags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoHostExtendX;
                            if (ImGui::BeginTable(func_var.first.c_str( ), 3, table_flags)) {
                                ImGui::TableSetupColumn("Var", ImGuiTableColumnFlags_WidthFixed, 150.0f);
                                ImGui::TableSetupColumn("previous value", ImGuiTableColumnFlags_WidthFixed, 130.0f);
                                ImGui::TableSetupColumn("current value", ImGuiTableColumnFlags_WidthFixed, 130.0f);
                                ImGui::TableHeadersRow( );
                                for (int row = 0; row < func_var.second.size( ); row++) {
                                    ImGui::TableNextRow( );
                                    ImGui::TableSetColumnIndex(0);
                                    ImGui::Text(var_ptr->strtable.var_str[func_var.second[row].get().var_index].c_str());

                                    // previous value
                                    ImGui::TableSetColumnIndex(1);
                                    ImGui::SetNextItemWidth(80);
                                    if(isHex) ImGui::Text("0x%08X", row);
                                    else 

                                    // current value
                                    char buf[260] = "";
                                    ImGui::TableSetColumnIndex(1);
                                    ImGui::SetNextItemWidth(80);
                                    ImGui::InputText("", buf, 10, ImGuiInputTextFlags_CharsDecimal);
                                    // ImGui::Text("Row %d Column %d", row, column);
                                    }
                                }
                                ImGui::EndTable( );
                                ImGui::PopStyleVar( );
                            }
                            ImGui::TreePop( );
                        }
                        var_table_idx++;
                    }
                    ImGui::EndChild( );
                    ImGui::EndTabItem( );
                    if (ImGui::BeginTabItem("pinned")) {
                        ImGui::Text("This is the Broccoli tab!\nblah blah blah blah blah");
                        ImGui::EndTabItem( );
                    }
                    ImGui::EndTabBar( );
                }
            }
            ImGui::EndChild( );
            ImGui::Separator( );
            ImGui::End( );
        }

    //  end_signature( );
    // End the window
    ImGui::End( );
    ///////////////////////////////////////////////////////////////////////////////
}

void Debug_UI() {

}
