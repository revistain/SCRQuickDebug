#ifndef DRAWING_H
#define DRAWING_H
#include "./dependencies/imgui/imgui.h"
#include "./dependencies/imgui/imgui_impl_dx11.h"
#include "./dependencies/imgui/imgui_impl_win32.h"
#include <d3d11.h>
#include <tchar.h>

#include "./console/console.hpp"
#include "ratio.h"

void DrawSquare(float x, float y, float sizeX, float sizeY);
void DrawSquareWithLabel(float x, float y, float sizeX, float sizeY, const char* label);

#endif