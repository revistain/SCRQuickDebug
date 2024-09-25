#ifndef DLLMAIN_H
#define DLLMAIN_H
#include <Windows.h>
#include <iostream>

HINSTANCE getDllHandle( );
DWORD WINAPI OnProcessDetach(LPVOID lpParam);

#endif