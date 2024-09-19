#include "imgui_scr.h"
#include "memoryRW.h"
#include "debugger_main.h"
#include "console/console.hpp"
#include <memory>

// Global variables
ID3D11Device* g_pd3dDevice = NULL;
ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
std::unique_ptr<Variables> var_ptr;

void addLocation(float mapX, float mapY, float sizeX, float sizeY, const char* label) {

}

GameData findProcessData() {
    OpenTargetProcess( );
    uint32_t exeAddr = getEXEAddr( );
    GameData gdata;
    gdata.screen_size_x  = Internal::dwread(exeAddr + 0xB31AC0);
    gdata.screen_size_y  = Internal::dwread(exeAddr + 0xB31AC8);
    gdata.console_height = Internal::dwread(exeAddr + 0xB31AC4);

    CloseTargetProcess( );
    return gdata;
}

void onImguiStart() {
    if (var_ptr) return;
    LOG("=========== var_ptr: 0x%08X =============\n", var_ptr.get());
    var_ptr = std::make_unique<Variables>(getVariables());
}

// Your own window and controls
void StarCraft_UI() {
    ImVec2 screenSize = ImGui::GetIO().DisplaySize;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_AlwaysAutoResize |
                                    ImGuiWindowFlags_NoBackground;

    // Set the window size to full screen
    ImGui::SetNextWindowSize(screenSize);
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    ///////////////////////////////////////////////////////////////////////////////
    // Begin the window
    ImGui::Begin("My Full-Sized Window", nullptr, window_flags);
    onImguiStart();

    GameData game_data = findProcessData();
    var_ptr->update_value( );
    uint32_t top  = var_ptr->screenTL[0];
    uint32_t left = var_ptr->screenTL[1];

    // Set the background color to fully transparent
    // ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0)); // Fully transparent
    ImGui::Text("Hello, World! %d %d", top, left);                                 // You can add other UI elements here
    if (ImGui::Button("Click Me")) {
        // Button action
    }
    ImGui::PopStyleColor( ); // Restore previous style

    // End the window
    ImGui::End( );
    ///////////////////////////////////////////////////////////////////////////////
}
