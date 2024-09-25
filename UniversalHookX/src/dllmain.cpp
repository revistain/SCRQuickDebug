#include <Windows.h>
#include <iostream>
#include <thread>

#include "console/console.hpp"

#include "hooks/hooks.hpp"
#include "utils/utils.hpp"

#include "dependencies/minhook/MinHook.h"
#include "ratio.h"
#include "./menu/menu.hpp"
#include "dllmain.h"
#include "imgui_scr.h"

DWORD WINAPI OnProcessAttach(LPVOID lpParam);
DWORD WINAPI OnProcessDetach(LPVOID lpParam);

HINSTANCE g_hInstance = NULL;
HINSTANCE getDllHandle( ) { return g_hInstance; }

DWORD WINAPI KeyPressListener(LPVOID lpParam) {
    while (true) {
        if (isExit() || (GetAsyncKeyState(VK_F9) & 0x8000)) {
            FreeLibraryAndExitThread(static_cast<HMODULE>(lpParam), 0);
        }

        Sleep(100);
    }
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        g_hInstance = hinstDLL;

        U::SetRenderingBackend(DIRECTX11);

        HANDLE hHandle = CreateThread(NULL, 0, OnProcessAttach, hinstDLL, 0, NULL);
        if (hHandle != NULL) {
            CloseHandle(hHandle);
        }
        HANDLE hKeyHandle = CreateThread(NULL, 0, KeyPressListener, hinstDLL, 0, NULL);
        if (hKeyHandle != NULL) {
            CloseHandle(hKeyHandle);
        }
    } else if (fdwReason == DLL_PROCESS_DETACH && !lpReserved) {
        OnProcessDetach(NULL);
    }

    return TRUE;
}


DWORD WINAPI OnProcessAttach(LPVOID lpParam) {
    Console::Alloc( );
    LOG("[+] Rendering backend: %s\n", U::RenderingBackendToStr( ));
    if (U::GetRenderingBackend( ) == NONE) {
        LOG("[!] Looks like you forgot to set a backend. Will unload after pressing enter...");
        std::cin.get( );

        FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(lpParam), 0);
        return 0;
    }

    MH_Initialize( );
    H::Init( );

    return 0;
}

DWORD WINAPI OnProcessDetach(LPVOID lpParam) {
    H::Free( );
    MH_Uninitialize( );

    Console::Free( );

    return 0;
}
