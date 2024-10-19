#include <Windows.h>
#include <iostream>
#include <thread>
#include <detours.h>
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
        if ((GetAsyncKeyState(VK_F12) )) {
            std::string callstack = getCallStack( );
            int result = MessageBoxA(
                NULL,                                                        // 부모 윈도우 핸들(NULL이면 최상위 윈도우)
                std::format("SC:R Crash CallStack\n{}", callstack).c_str( ), // 메시지 내용
                "QuickDebug CallStack popup",                                   // 메시지 상자 제목
                MB_OK | MB_ICONQUESTION                                      // 확인 및 취소 버튼과 아이콘
            );
        }
        Sleep(100);
    }
    return 0;
}

BOOL(WINAPI* OriginalTerminateProcess)
(HANDLE, UINT) = TerminateProcess;

// 후킹된 TerminateProcess 함수
DWORD WINAPI callCrashMessageBox(LPVOID lpParam) {
    std::string callstack = getCallStack( );
    int result = MessageBoxA(
        NULL,                                                        // 부모 윈도우 핸들(NULL이면 최상위 윈도우)
        std::format("SC:R Crash CallStack\n{}", callstack).c_str( ), // 메시지 내용
        "SC:R Crashed popup",                                        // 메시지 상자 제목
        MB_OK | MB_ICONQUESTION                                      // 확인 및 취소 버튼과 아이콘
    );
    return result;
}
BOOL WINAPI HookedTerminateProcess(HANDLE hProcess, UINT uExitCode) {
    //
    std::cout << "TerminateProcess가 호출되었습니다! 종료 코드: " << uExitCode << std::endl;
    std::string callstack = getCallStack( );
    int result = MessageBoxA(
        NULL,                                                        // 부모 윈도우 핸들(NULL이면 최상위 윈도우)
        std::format("SC:R Crash CallStack\n{}", callstack).c_str( ), // 메시지 내용
        "SC:R Crashed popup",                                        // 메시지 상자 제목
        MB_OK | MB_ICONQUESTION                                      // 확인 및 취소 버튼과 아이콘
    );
    return result;
    return OriginalTerminateProcess(hProcess, uExitCode);
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
        // TerminateProcess 함수 후킹
        DetourTransactionBegin( );
        DetourUpdateThread(GetCurrentThread( ));
        DetourAttach(&(PVOID&)OriginalTerminateProcess, HookedTerminateProcess);
        DetourTransactionCommit( );
    }
    else if (fdwReason == DLL_PROCESS_DETACH) {
        DetourTransactionBegin( );
        DetourUpdateThread(GetCurrentThread( ));
        DetourDetach(&(PVOID&)OriginalTerminateProcess, HookedTerminateProcess);
        DetourTransactionCommit( );
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
