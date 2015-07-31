#include "libmain.hpp"

#include "tools/common.hpp"
#include "d3d_proxy.hpp"
#include "settings.hpp"

typedef IDirect3D9* (WINAPI* Direct3DCreate9_Type)(UINT);

logger*   gLog      = nullptr;
settings* gSettings = nullptr;

HINSTANCE           hOriginalDll  = nullptr;
HINSTANCE           hThisInstance = nullptr;

Direct3DCreate9_Type Direct3DCreate9_fn = nullptr;

#ifdef __GNUC__
extern "C" D3D9_EXPORT
#endif
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    (void)lpReserved;
    bool result = true;
    switch(ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH: result = InitInstance(hModule); break;
        case DLL_PROCESS_DETACH: ExitInstance(); break;

        case DLL_THREAD_ATTACH:  break;
        case DLL_THREAD_DETACH:  break;
    }
    return result;
}

IDirect3D9* Direct3DCreate9(UINT version)
{
    LOG_STATIC_FUNCTION();
    if(!hOriginalDll)
    {
        if(!LoadOriginalDll())
        {
            return nullptr;
        }
    }
    IDirect3D9* ret = (Direct3DCreate9_fn(version));
    if(nullptr == ret)
    {
        LOG_ERROR() << "Unable to create d3d9";
        return nullptr;
    }
    return new d3d_proxy(ret, gSettings);
}

bool InitInstance(HANDLE hModule)
{
    hOriginalDll  = nullptr;
    hThisInstance = (HINSTANCE)hModule;

    if(nullptr == gSettings)
    {
        gSettings = new settings("./d3d9.ini");
    }
    if(nullptr == gLog)
    {
        gLog = new logger(gSettings->logLevel(), false, true);
        gLog->setFile("./" + gSettings->logFile());
    }
    LOG_INFO() << FUNC_NAME;
    return true;
}

bool LoadOriginalDll()
{
    LOG_STATIC_FUNCTION();
    char buffer[MAX_PATH];

    ::GetSystemDirectoryA(buffer, MAX_PATH);
    strcat(buffer,"/d3d9.dll");

    if(nullptr == hOriginalDll) hOriginalDll = ::LoadLibraryA(buffer);
    if(nullptr == hOriginalDll)
    {
        LOG_ERROR() << "LoadLibrary failed: " << getWinError();
        return false;
    }

    Direct3DCreate9_fn = (Direct3DCreate9_Type)::GetProcAddress(hOriginalDll, "Direct3DCreate9");
    if(nullptr == Direct3DCreate9_fn)
    {
        LOG_ERROR() << "Unable to get \"Direct3DCreate9\": " << getWinError();
        return false;
    }
    return true;
}

void ExitInstance()
{
    LOG_INFO() << FUNC_NAME;
    if(nullptr != hOriginalDll)
    {
        ::FreeLibrary(hOriginalDll);
        hOriginalDll = nullptr;
    }
    hThisInstance = nullptr;

    delete gSettings;
    gSettings = nullptr;
    delete gLog;
    gLog = nullptr;
}
