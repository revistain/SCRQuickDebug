#include "pch.h"
#include "glhook.h"
#include <GL/gl.h>
#include <windows.h>
#include <iostream>
#include <thread>
#include <fstream>
#include <string>

std::thread* readingThread = nullptr;
bool running = true; // Control the running state of the thread

// Function to read a uint16_t value from memory
extern "C" __declspec(dllexport) bool ReadUInt16Value(LPCVOID baseAddress, uint16_t * outValue) {
    SIZE_T bytesRead;
    return ReadProcessMemory(GetCurrentProcess(), baseAddress, outValue, sizeof(uint16_t), &bytesRead) && bytesRead == sizeof(uint16_t);
}

// Thread function to read memory periodically
void MemoryReadingThread() {
    while (running) {
        uint16_t value = 0;
        LPCVOID targetAddress = (LPCVOID)0xea9910; // Example address, change to the actual address
        if (ReadUInt16Value(targetAddress, &value)) {
            std::wstring message = L"Read value: " + std::to_wstring(value);
            MessageBox(NULL, message.c_str(), L"Memory Read Result", MB_OK | MB_ICONINFORMATION);
        }
        else {
            MessageBox(NULL, L"Failed to read memory.", L"Memory Read Error", MB_OK | MB_ICONERROR);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // Approx 60 FPS
    }
}
///////////////////////// hook drawing
typedef void (APIENTRY* glBegin_t)(GLenum mode);
glBegin_t original_glBegin = nullptr;

// Example of a hooked OpenGL function
void glBeginHook(GLenum mode) {
    // Call the original function
    original_glBegin(mode);

    // Custom drawing code
    if (mode == GL_TRIANGLES) {
        // Your drawing code here
        glColor3f(1.0f, 0.0f, 0.0f); // Set color to red
        glVertex3f(0.0f, 1.0f, 0.0f); // Vertex 1
        glVertex3f(-1.0f, -1.0f, 0.0f); // Vertex 2
        glVertex3f(1.0f, -1.0f, 0.0f); // Vertex 3
    }
}


typedef void(__stdcall* glFinish_t)();
glFinish_t o_glFinish;

void __stdcall h_glFinish()
{
    MessageBox(0, L"glFinish hooked!", 0, 0);
    o_glFinish();
}

// call from inside the main thread (e.g. in a LoadLibrary hook). Note there are other
// ways of getting the desired threads local storage base that are more stealthy than getting
// fs:[18] value from inside the rendering thread.


HHOOK hHook;
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pKbd = (KBDLLHOOKSTRUCT*)lParam;
        if (wParam == WM_KEYDOWN) {
            if (pKbd->vkCode == VK_SPACE) {
                char func_name[] = "glFinish";
                if (o_glFinish) GLUnhook(func_name, o_glFinish);
            }
        }
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void SetHook() {
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
}

void Unhook() {
    UnhookWindowsHookEx(hHook);
}
// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        SetHook();
        original_glBegin = (glBegin_t)wglGetProcAddress("glBegin");
        MessageBox(0, L"glFinish hooked!1", 0, 0);
        if (GLInit()) {
            char fname[] = "glFinish";
            o_glFinish = (glFinish_t)GLHook(fname, h_glFinish);
        }

        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        // Clean up for thread detach (if needed)
        break;

    case DLL_PROCESS_DETACH:
        Unhook();
        break;
    }
    return TRUE;
}
