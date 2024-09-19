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

/*
* int getWindowWidth( ) {
    if (cachedhwnd) {
        RECT rect;
        if (GetClientRect(cachedhwnd, &rect)) {
            int left = rect.left;
            int right = rect.right;
            int width = right - left;
            return width;
        }
    }
    return 0;
}

uint32_t getPillar(uint32_t width, uint32_t ratio) {
    getHWND( );
    uint32_t win_width = getWindowWidth( );
    std::cout << "winwidth: " << win_width << " width: " << width << "\n";
    return (win_width - width) / 2;
}
*/
