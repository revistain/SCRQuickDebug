#pragma once

#include <Windows.h>
bool isExit( );
namespace Menu {
    void InitializeContext(HWND hwnd);
    void Render( );

    inline bool bShowMenu = true;
} // namespace Menu
