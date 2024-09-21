#include <Windows.h>
#include <iostream>
#include <thread>

#include "console/console.hpp"

#include "hooks/hooks.hpp"
#include "utils/utils.hpp"

#include "dependencies/minhook/MinHook.h"
#include "ratio.h"

DWORD WINAPI OnProcessAttach(LPVOID lpParam);
DWORD WINAPI OnProcessDetach(LPVOID lpParam);

#include <Windows.h>
#include <iostream>

HINSTANCE g_hInstance = NULL;

// 키보드 입력 감지 스레드
DWORD WINAPI KeyPressListener(LPVOID lpParam) {
    while (true) {
        // '-' 키를 감지 (VK_OEM_MINUS는 '-'의 가상 키 코드)
        if (GetAsyncKeyState(VK_OEM_MINUS) & 0x8000) {
            std::cout << "'-' 키가 눌렸습니다. DLL을 분리합니다.\n";
            FreeLibraryAndExitThread(static_cast<HMODULE>(lpParam), 0);
        }

        // 너무 자주 체크하지 않도록 약간의 딜레이를 줍니다
        Sleep(100);
    }
    return 0;
}

// DLL 메인 함수
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        g_hInstance = hinstDLL;

        // 렌더링 백엔드 설정 (예시)
        U::SetRenderingBackend(DIRECTX11);

        HANDLE hHandle = CreateThread(NULL, 0, OnProcessAttach, hinstDLL, 0, NULL);
        if (hHandle != NULL) {
            CloseHandle(hHandle);
        }
        // 키 입력 감지 스레드 생성
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
