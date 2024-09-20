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

std::string W(const std::wstring& wideStr) {
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str( ), -1, nullptr, 0, nullptr, nullptr);
    std::string utf8Str(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str( ), -1, &utf8Str[0], sizeNeeded, nullptr, nullptr);
    return utf8Str;
}

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
    }
    loc_ptr = std::make_unique<Locations>(findMRGNAddr("GongNkdfhLpZmqWnRbZlfhInbpQYtZBwjeOqmPlW"), var_ptr->Locations);
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

    ImFont* pfont = getFont(3);
    if (pfont)
        ImGui::PushFont(pfont);
    ImGui::PushFont(pfont);
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
    ImGui::PopStyleColor( ); // Restore previous style

    ////////////
    ImGuiWindowFlags main_window_flags = ImGuiWindowFlags_AlwaysAutoResize;
    ImGui::Begin("SC:R Debug Window", nullptr, main_window_flags);
    
    if (ImGui::Button("Open New Window")) {
        starcraft_input = false;
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
    if (pfont) ImGui::PopFont( );
    ImGui::End( );
    ///////////////////////////////////////////////////////////////////////////////
}

void Debug_UI() {

}
