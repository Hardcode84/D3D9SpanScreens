#ifndef D3D_PROXY_HPP
#define D3D_PROXY_HPP

#include <d3d9.h>
#include <atomic>
#include <vector>

#include "tools/common.hpp"
#include "tools/com_ptr.hpp"

#define D3D_API __stdcall

class d3ddevice_proxy;
class settings;

class d3d_proxy : public IDirect3D9
{
    std::atomic<ULONG> mRefCount;
    com_ptr<IDirect3D9> mD3D;
    unsigned mMonCount = 0;
    bool mIsHorizontal = true;
    std::vector<int> mScreensMapping;
    bool mRestoreState = false;

    virtual ~d3d_proxy();
public:
    d3d_proxy(IDirect3D9* d3d, const settings* set);


    bool useHack()      const { return (mMonCount > 1); }
    bool isHorizontal() const { return mIsHorizontal; }
    std::vector<int> getScreensMapping() const { return mScreensMapping; }
    bool restoreStates() const { return mRestoreState; }

    unsigned getNumMonitors() const { return mMonCount; }

    HRESULT D3D_API QueryInterface(const IID &riid, LPVOID *ppvObj);
    ULONG   D3D_API AddRef();
    ULONG   D3D_API Release();

    //IDirect3D9
    HRESULT D3D_API RegisterSoftwareDevice(void *pInitializeFunction);
    UINT    D3D_API GetAdapterCount();
    HRESULT D3D_API GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9 *pIdentifier);
    UINT    D3D_API GetAdapterModeCount(UINT Adapter, D3DFORMAT Format);
    HRESULT D3D_API EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE *pMode);
    HRESULT D3D_API GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode);
    HRESULT D3D_API CheckDeviceType(UINT iAdapter, D3DDEVTYPE DevType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, WINBOOL bWindowed);
    HRESULT D3D_API CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat);
    HRESULT D3D_API CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, WINBOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD *pQualityLevels);
    HRESULT D3D_API CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat);
    HRESULT D3D_API CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat);
    HRESULT D3D_API GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9 *pCaps);
    HMONITOR D3D_API GetAdapterMonitor(UINT Adapter);
    HRESULT D3D_API CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice9 **ppReturnedDeviceInterface);

};

#endif // D3D_PROXY_HPP
