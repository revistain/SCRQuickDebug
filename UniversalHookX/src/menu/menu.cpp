#include "menu.hpp"

#include "../dependencies/imgui/imgui.h"
#include "../dependencies/imgui/imgui_impl_win32.h"

#include "../imgui_scr.h"
#include "../drawing.h"
#include "../font.h"
namespace ig = ImGui;

extern "C" __declspec(dllexport) void SendErrorToQuickDebug( ) {
    HWND hwnd = FindWindowA(NULL, "QuickDebug");
    if (hwnd != NULL) {
        SendMessage(hwnd, 0xEDACEDAC, 0, 0);
    }
}

namespace Menu {
    void InitializeContext(HWND hwnd) {
        if (ig::GetCurrentContext( ))
            return;

        ImGui::CreateContext( );
        initFonts( );
        ImGui_ImplWin32_Init(hwnd);

        ImGuiIO& io = ImGui::GetIO( );
        io.IniFilename = io.LogFilename = nullptr;
    }

    void Render( ) {
        try {
            if (!bShowMenu)
                return;

            StarCraft_UI( );
            // ig::ShowDemoWindow( );
        }
        catch (const char* e) {
            SendErrorToQuickDebug( );
            int size_needed = MultiByteToWideChar(CP_UTF8, 0, e, (int)strlen(e), NULL, 0);
            std::wstring wideErrorMsg(size_needed, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, e, (int)strlen(e), &wideErrorMsg[0], size_needed);

            std::wstring message = std::format(L"Error On QuickDebug\n[ {} ]", wideErrorMsg);
            MessageBox(NULL, message.c_str( ), L"QuickDebug Crashed", MB_OK);
            throw e;
        }
    }
} // namespace Menu
