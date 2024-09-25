#include "menu.hpp"

#include "../dependencies/imgui/imgui.h"
#include "../dependencies/imgui/imgui_impl_win32.h"

#include "../imgui_scr.h"
#include "../drawing.h"
#include "../font.h"

bool SendMessageToQuickDebug(UINT messageCode, const std::wstring& message) {
    // Find the window handle for the process named "QuickDebug"
    HWND hwnd = FindWindow(NULL, L"QuickDebug"); // Window title must match

    if (hwnd == NULL) {
        std::wcout << L"Could not find the QuickDebug window." << std::endl;
        return false; // Error: window not found
    }

    // Send the message
    WPARAM wParam = 0;                                          // Example wParam, can be modified as needed
    LPARAM lParam = reinterpret_cast<LPARAM>(message.c_str( )); // Convert message to LPARAM

    LRESULT result = PostMessage(hwnd, messageCode, wParam, lParam);

    // Check the result
    if (result == 0) {
        std::wcout << L"SendMessage failed." << std::endl;
        return false;
    }

    std::wcout << L"Message sent successfully." << std::endl;
    return true;
}

namespace ig = ImGui;
const uint32_t VERSION_MAIN = 0;
const uint32_t VERSION_MID = 0;
const uint32_t VERSION_END = 1;
static bool detachFlag = false;
bool isExit( ) { return detachFlag; }
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
            if (str.contains("qdebug Version mismatch!")) {
                SendMessageToQuickDebug(WM_USER + 2, L"");
            }
            else if (str.contains("Pattern not found")) {
                SendMessageToQuickDebug(WM_USER + 1, L"");
            }
            detachFlag = true;
            std::wstring wideStr(str.begin( ), str.end( ));
            auto message = std::format(L"Error On QuickDebug\n====================\n{}\n====================\n*debug_dll version: {}.{}.{}",
                                       wideStr, VERSION_MAIN, VERSION_MID, VERSION_END);
            MessageBoxW(NULL, message.c_str(), L"Crash!", MB_OK);
        }
    }
} // namespace Menu
