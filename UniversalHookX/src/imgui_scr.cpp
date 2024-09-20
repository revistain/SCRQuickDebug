#include "imgui_scr.h"
#include "memoryRW.h"
#include "debugger_main.h"
#include "signature.h"
#include "console/console.hpp"
#include <memory>

// Global variables
ID3D11Device* g_pd3dDevice = NULL;
ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
std::unique_ptr<Variables> var_ptr;
std::unique_ptr<Locations> loc_ptr;


void DrawLocations(GameData game_data) {
    uint32_t screen_top  = var_ptr->screenTL[0];
    uint32_t screen_left = var_ptr->screenTL[1];

    //var_ptr->Locations
    //addLocation(screen_top, ;

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
    initFonts( );
    if (OpenTargetProcess( )) {
        GetModuleBaseAddress(L"StarCraft.exe");
    }
    loc_ptr = std::make_unique<Locations>(findMRGNAddr("GongNkdfhLpZmqWnRbZlfhInbpQYtZBwjeOqmPlW"), var_ptr->Locations);
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
    //////////////// initialize ////////////////
    onImguiStart();

    ////////////////   update   ////////////////
    var_ptr->update_value( );
    GameData game_data = updateGameData( );
    loc_ptr->drawLocations(var_ptr, game_data);

    ////////////////    loop    ////////////////
    DrawLocations(game_data);
    

    // Set the background color to fully transparent
    // ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0)); // Fully transparent
    ImGui::Text("Hello, World!");                                 // You can add other UI elements here
    if (ImGui::Button("Click Me")) {
        // Button action
    }
    ImGui::PopStyleColor( ); // Restore previous style


    //  end_signature( );
    // End the window
    ImGui::End( );
    ///////////////////////////////////////////////////////////////////////////////
}
