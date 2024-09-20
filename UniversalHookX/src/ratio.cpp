#include "ratio.h"

HWND cachedhwnd = NULL;
void getHWND() {
    // Find the window by title
    if (!cachedhwnd)
        cachedhwnd = FindWindowA(NULL, "Brood War");
    else
        return;
}

int getWindowHeight() {
    if (cachedhwnd) {
        RECT rect;
        if (GetClientRect(cachedhwnd, &rect)) {
            int top = rect.top;
            int bottom = rect.bottom;
            int height = bottom - top;
            return height;
        }
    }
    return 0;
}

float getRatio() {
    getHWND();
    int height = getWindowHeight();
    return (float)height / 480;
}
