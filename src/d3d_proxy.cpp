#include "d3d_proxy.hpp"

#include "d3ddevice_proxy.hpp"
#include "settings.hpp"

d3d_proxy::d3d_proxy(IDirect3D9* d3d, const settings* set):
    mRefCount(1),
    mD3D(d3d),
    mIsHorizontal(set->isHorizontal()),
    mScreensMapping(set->getScreensMapping()),
    mRestoreState(set->restoreStates())
{
    LOG_FUNCTION();
    mMonCount = d3d->GetAdapterCount();
    WRITE_VAR(mMonCount);
}

d3d_proxy::~d3d_proxy()
{
    LOG_FUNCTION();
}

HRESULT D3D_API d3d_proxy::QueryInterface(const IID &riid, LPVOID *ppvObj)
{
    LOG_FUNCTION();
    return mD3D->QueryInterface(riid,ppvObj);
}

ULONG   D3D_API d3d_proxy::AddRef()
{
    return ++mRefCount;
}

ULONG   D3D_API d3d_proxy::Release()
{
    const ULONG ret = --mRefCount;
    if(0 == ret)
    {
        delete this;
        return 0;
    }
    return ret;
}

//IDirect3D9
HRESULT D3D_API d3d_proxy::RegisterSoftwareDevice(void *pInitializeFunction)
{
    LOG_FUNCTION();
    return mD3D->RegisterSoftwareDevice(pInitializeFunction);
}
UINT    D3D_API d3d_proxy::GetAdapterCount()
{
    LOG_FUNCTION();
    if(useHack()) { return 1; }
    return mD3D->GetAdapterCount();
}
HRESULT D3D_API d3d_proxy::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9 *pIdentifier)
{
    LOG_FUNCTION();
    return mD3D->GetAdapterIdentifier(Adapter,Flags,pIdentifier);
}
UINT    D3D_API d3d_proxy::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format)
{
    LOG_FUNCTION();
    return mD3D->GetAdapterModeCount(Adapter, Format);
}
HRESULT D3D_API d3d_proxy::EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE *pMode)
{
    LOG_FUNCTION();
    const auto hr = mD3D->EnumAdapterModes(Adapter, Format, Mode, pMode);
    if(useHack() && SUCCEEDED(hr))
    {
        if(isHorizontal())
        {
            pMode->Width *= mMonCount;
        }
        else
        {
            pMode->Height *= mMonCount;
        }
    }
    return hr;
}
HRESULT D3D_API d3d_proxy::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode)
{
    LOG_FUNCTION();
    const auto hr = mD3D->GetAdapterDisplayMode(Adapter,pMode);
    if(useHack() && SUCCEEDED(hr))
    {
        if(isHorizontal())
        {
            pMode->Width *= mMonCount;
        }
        else
        {
            pMode->Height *= mMonCount;
        }
    }
    return hr;
}
HRESULT D3D_API d3d_proxy::CheckDeviceType(UINT iAdapter, D3DDEVTYPE DevType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, WINBOOL bWindowed)
{
    LOG_FUNCTION();
    return mD3D->CheckDeviceType(iAdapter, DevType, DisplayFormat, BackBufferFormat, bWindowed);
}
HRESULT D3D_API d3d_proxy::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
    //LOG_FUNCTION();
    const auto hr = mD3D->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
    /*
    if(FAILED(hr))
    {
        LOG_WARNING() << "Format unsupported: " << getWinErrorFromHr(hr);
        WRITE_VAR(Adapter);
        WRITE_VAR(DeviceType);
        WRITE_VAR(AdapterFormat);
        WRITE_VAR(RType);
        WRITE_VAR(CheckFormat);
    }
    */
    return hr;
}
HRESULT D3D_API d3d_proxy::CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, WINBOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD *pQualityLevels)
{
    LOG_FUNCTION();
    return mD3D->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}
HRESULT D3D_API d3d_proxy::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
    LOG_FUNCTION();
    return mD3D->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}
HRESULT D3D_API d3d_proxy::CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat)
{
    LOG_FUNCTION();
    return mD3D->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
}
HRESULT D3D_API d3d_proxy::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9 *pCaps)
{
    LOG_FUNCTION();
    const auto hr = mD3D->GetDeviceCaps(Adapter, DeviceType, pCaps);
    if(useHack() && SUCCEEDED(hr))
    {
        pCaps->AdapterOrdinalInGroup = 0;
        pCaps->NumberOfAdaptersInGroup = 1;
    }
    return hr;
}
HMONITOR D3D_API d3d_proxy::GetAdapterMonitor(UINT Adapter)
{
    LOG_FUNCTION();
    return mD3D->GetAdapterMonitor(Adapter);
}
HRESULT D3D_API d3d_proxy::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice9 **ppReturnedDeviceInterface)
{
    LOG_FUNCTION();
    if(!useHack() || pPresentationParameters->Windowed)
    {
        return mD3D->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters,ppReturnedDeviceInterface);
    }
    d3ddevice_proxy* dev;
    const auto hr = d3ddevice_proxy::createDeviceProxy(this,
                                                       mD3D.get(),
                                                       Adapter,
                                                       DeviceType,
                                                       hFocusWindow,
                                                       BehaviorFlags,
                                                       pPresentationParameters,
                                                       &dev);
    if(SUCCEEDED(hr))
    {
        *ppReturnedDeviceInterface = dev;
    }
    else
    {
        *ppReturnedDeviceInterface = nullptr;
    }
    return hr;
}

