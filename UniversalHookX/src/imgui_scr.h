#ifndef IMGUI_SCR_H
#define IMGUI_SCR_H
#include "./dependencies/imgui/imgui.h"
#include "./dependencies/imgui/imgui_impl_dx11.h"
#include "./dependencies/imgui/imgui_impl_win32.h"
#include <d3d11.h>
#include <tchar.h>

#include "./console/console.hpp"
#include "ratio.h"
#include "drawing.h"
#include "game_data.h"
#include "LocationDraw.h"
#include "cunit.h"

void StarCraft_UI( );
void setExit( );
bool isExit( );
#endif
