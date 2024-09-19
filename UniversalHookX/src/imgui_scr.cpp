#include "imgui_scr.h"
#include "debugger_main.h"

// Global variables
ID3D11Device* g_pd3dDevice = NULL;
ID3D11DeviceContext* g_pd3dDeviceContext = NULL;

void addLocation(float mapX, float mapY, float sizeX, float sizeY, const char* label) {


}

// Your own window and controls
void StarCraft_UI() {
    // Get the screen size (assuming you have access to the window handle)
    ImVec2 screenSize = ImGui::GetIO( ).DisplaySize;

    // Set window flags
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_AlwaysAutoResize |
                                    ImGuiWindowFlags_NoBackground;

    // Set the window size to full screen
    ImGui::SetNextWindowSize(screenSize);
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    ////////////////////////////////////////////////////
    // Begin the window
    ImGui::Begin("My Full-Sized Window", nullptr, window_flags);
    debugger_main();


    // Set the background color to fully transparent
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0)); // Fully transparent
    ImGui::Text("Hello, World!");                                 // You can add other UI elements here
    if (ImGui::Button("Click Me")) {
        // Button action
    }
    ImGui::PopStyleColor( ); // Restore previous style

    // End the window
    ImGui::End( );
    ////////////////////////////////////////////////////
}
