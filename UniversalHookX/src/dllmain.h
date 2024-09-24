#ifndef DLLMAIN_H
#define DLLMAIN_H
#include <Windows.h>
#include <iostream>

HINSTANCE getDllHandle( );
void detachSelf( );
DWORD WINAPI OnProcessDetach(LPVOID lpParam);
void setExit( );

#endif