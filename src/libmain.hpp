#ifndef LIBMAIN_HPP
#define LIBMAIN_HPP


#include "d3d9_export.h"
#include <windows.h>
#include <d3d9.h>

extern "C"
{
D3D9_EXPORT IDirect3D9* Direct3DCreate9(UINT version);
}

bool InitInstance(HANDLE hModule);
void ExitInstance();
bool LoadOriginalDll();

#endif // LIBMAIN_HPP
