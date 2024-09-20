#include "menu.hpp"

#include "../dependencies/imgui/imgui.h"
#include "../dependencies/imgui/imgui_impl_win32.h"

#include "../imgui_scr.h"
#include "../drawing.h"
namespace ig = ImGui;

namespace Menu {
    void InitializeContext(HWND hwnd) {
        if (ig::GetCurrentContext( ))
            return;

        ImGui::CreateContext( );
        ImGui_ImplWin32_Init(hwnd);
        initFonts( );

        ImGuiIO& io = ImGui::GetIO( );
        io.IniFilename = io.LogFilename = nullptr;
    }

    void Render( ) {
        if (!bShowMenu)
            return;
        
        Debug_UI( );
        StarCraft_UI( );
        //ig::ShowDemoWindow( );
    }
} // namespace Menu
