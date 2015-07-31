#include "d3dswapchain_proxy.hpp"

#include "d3ddevice_proxy.hpp"

d3dswapchain_proxy::d3dswapchain_proxy(d3ddevice_proxy* parent,
                                       IDirect3DSwapChain9* swapChain,
                                       unsigned index):
    mRefCount(1),
    mSC(swapChain),
    mParent(parent),
    mIndex(index)
{
    LOG_FUNCTION();
}

d3dswapchain_proxy::~d3dswapchain_proxy()
{
    LOG_FUNCTION();
}

HRESULT D3D_API d3dswapchain_proxy::QueryInterface(const IID &riid, LPVOID *ppvObj)
{
    LOG_FUNCTION();
    return mSC->QueryInterface(riid,ppvObj);
}

ULONG   D3D_API d3dswapchain_proxy::AddRef()
{
    return ++mRefCount;
}

ULONG   D3D_API d3dswapchain_proxy::Release()
{
    const ULONG ret = --mRefCount;
    if(0 == ret)
    {
        delete this;
        return 0;
    }
    return ret;
}

//IDirect3DSwapChain9
HRESULT D3D_API d3dswapchain_proxy::Present(const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion, DWORD dwFlags)
{
    return mParent->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT D3D_API d3dswapchain_proxy::GetFrontBufferData(IDirect3DSurface9 *pDestSurface)
{
    return mParent->GetFrontBufferData(mIndex, pDestSurface);
}

HRESULT D3D_API d3dswapchain_proxy::GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9 **ppBackBuffer)
{
    return mParent->GetBackBuffer(mIndex, iBackBuffer, Type, ppBackBuffer);
}

HRESULT D3D_API d3dswapchain_proxy::GetRasterStatus(D3DRASTER_STATUS *pRasterStatus)
{
    return mSC->GetRasterStatus(pRasterStatus);
}

HRESULT D3D_API d3dswapchain_proxy::GetDisplayMode(D3DDISPLAYMODE *pMode)
{
    return mParent->GetDisplayMode(mIndex, pMode);
}

HRESULT D3D_API d3dswapchain_proxy::GetDevice(IDirect3DDevice9 **ppDevice)
{
    CHECK_PARAM(nullptr != ppDevice);
    mParent->AddRef();
    *ppDevice = mParent;
    return S_OK;
}

HRESULT D3D_API d3dswapchain_proxy::GetPresentParameters(D3DPRESENT_PARAMETERS *pPresentationParameters)
{
    HRESULT hr;
    CHECK_COM_METHOD(mSC, GetPresentParameters, pPresentationParameters);
    pPresentationParameters->BackBufferWidth  = mParent->getRtWidth();
    pPresentationParameters->BackBufferHeight = mParent->getRtHeight();
    return hr;
}
