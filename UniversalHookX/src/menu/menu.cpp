#include "menu.hpp"

#include "../dependencies/imgui/imgui.h"
#include "../dependencies/imgui/imgui_impl_win32.h"

#include "../imgui_scr.h"
#include "../dllmain.h"
#include "../drawing.h"
#include "../font.h"

namespace ig = ImGui;
const uint32_t VERSION_MAIN = 0;
const uint32_t VERSION_MID = 0;
const uint32_t VERSION_END = 1;
namespace Menu {
    void InitializeContext(HWND hwnd) {
        if (ig::GetCurrentContext( ))
            return;

        ImGui::CreateContext( );
        // initFonts( );
        ImGui_ImplWin32_Init(hwnd);

        ImGuiIO& io = ImGui::GetIO( );
        io.IniFilename = io.LogFilename = nullptr;
    }

    void Render( ) {
        try {
            if (!bShowMenu)
                return;

            StarCraft_UI( );
        }
        catch (const std::string& str) {
            std::wstring wideStr(str.begin( ), str.end( ));
            auto message = new std::wstring(std::format(L"Error On QuickDebug\n====================\n{}\n====================\n*debug_dll version: {}.{}.{}", wideStr, VERSION_MAIN, VERSION_MID, VERSION_END));

            setExit( );
        }
    }
} // namespace Menu
