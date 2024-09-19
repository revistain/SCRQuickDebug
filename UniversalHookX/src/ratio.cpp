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
        if (GetWindowRect(cachedhwnd, &rect)) {
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
    float height = getWindowHeight();
    return height / 480;
}
