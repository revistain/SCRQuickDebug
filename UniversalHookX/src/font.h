#ifndef FONT_H
#define FONT_H
#include <filesystem>
#include <iostream>
#include <windows.h>
#include "imgui_scr.h"
#include "../resource.h"

void initFonts( );
ImFont* getFont(int size);

#endif