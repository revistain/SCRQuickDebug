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
void StarCraft_UI() {
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
    ImGuiWindowFlags main_window_flags = ImGuiWindowFlags_AlwaysAutoResize;
    ImGui::Begin("SC:R Debug Window", nullptr, main_window_flags);
    static int currentPage = 0;
    ImGui::SameLine( );
    if (ImGui::Button("Inspect", ImVec2(150, 25))) {
        currentPage = 0;
    }
    ImGui::SameLine( );
    if (ImGui::Button("Trigger", ImVec2(150, 25))) {
        currentPage = 1;
    }
    ImGui::SameLine( );
    if (ImGui::Button("Settings", ImVec2(150, 25))) {
        currentPage = 2;
    }
    ImGui::Separator( );

    if (currentPage == 0) {
        static bool is_var_popup_open = false;
        if (ImGui::Button("Open EUDVariable inspector")) {
            is_var_popup_open = true;
        }
        if (is_var_popup_open) {
            ImGui::OpenPopup("EUDVariable Popup");
        }

        // 팝업이 열려 있다면 내용 표시
        if (ImGui::BeginPopup("EUDVariable Popup", ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("This is a popup.");

            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup( );
                is_var_popup_open = false;
            }

            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            if (ImGui::CollapsingHeader("Popup Content", ImGuiTreeNodeFlags_DefaultOpen)) {
                if (ImGui::BeginTabBar("EUDVariables tab", tab_bar_flags)) {
                    if (ImGui::BeginTabItem("EUDVariables")) {
                        ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
                        window_flags |= ImGuiWindowFlags_NoScrollWithMouse;
                        window_flags |= ImGuiWindowFlags_MenuBar;
                        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
                        ImGui::BeginChild("ChildR", ImVec2(480, 580), true, window_flags);
                        if (ImGui::BeginTable("table1", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoHostExtendX)) {
                            ImGui::TableSetupColumn("Var", ImGuiTableColumnFlags_WidthFixed, 150.0f);
                            ImGui::TableSetupColumn("previous value", ImGuiTableColumnFlags_WidthFixed, 130.0f);
                            ImGui::TableSetupColumn("current value", ImGuiTableColumnFlags_WidthFixed, 130.0f);
                            ImGui::TableHeadersRow( );
                            for (int row = 0; row < 4; row++) {
                                ImGui::TableNextRow( );
                                ImGui::TableSetColumnIndex(0);
                                ImGui::Text("next: ");
                                for (int column = 1; column < 3; column++) {
                                    char buf[260] = "0x";
                                    ImGui::TableSetColumnIndex(column);
                                    ImGui::SetNextItemWidth(80);
                                    ImGui::InputText("", buf, 10, ImGuiInputTextFlags_CharsDecimal);
                                    // ImGui::Text("Row %d Column %d", row, column);
                                }
                            }
                            ImGui::EndTable( );
                        }
                        ImGui::EndChild( );
                        ImGui::PopStyleVar( );
                        ImGui::EndTabItem( );
                    }
                    if (ImGui::BeginTabItem("pinned")) {
                        ImGui::Text("This is the Broccoli tab!\nblah blah blah blah blah");
                        ImGui::EndTabItem( );
                    }
                    if (ImGui::BeginTabItem("Cucumber")) {
                        ImGui::Text("This is the Cucumber tab!\nblah blah blah blah blah");
                        ImGui::EndTabItem( );
                    }
                    ImGui::EndTabBar( );
                }
            }
            ImGui::Separator( );
            ImGui::EndPopup( );
        }
    }

    if (ImGui::TreeNode("Location Setting")) { // Using a UTF-8 string
        // This creates a sub-menu node
        if (ImGui::TreeNode("Sub Menu")) {
            ImGui::Text(u8"Sub Item 1");
            ImGui::Text("Sub Item 2");

            // Create a nested sub-menu node
            if (ImGui::TreeNode("Nested Sub Menu")) {
                ImGui::Text("Nested Item 1");
                ImGui::Text("Nested Item 2");
                ImGui::TreePop( ); // Close the nested sub menu
            }

            ImGui::TreePop( ); // Close the sub menu
        }

        ImGui::TreePop( ); // Close the main menu
    }
    ImGui::End( );

    //  end_signature( );
    // End the window
    ImGui::End( );
    ///////////////////////////////////////////////////////////////////////////////
}

void Debug_UI() {

}
