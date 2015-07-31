#ifndef D3DSWAPCHAIN_PROXY_HPP
#define D3DSWAPCHAIN_PROXY_HPP

#include <d3d9.h>
#include <atomic>

#include "tools/common.hpp"
#include "tools/com_ptr.hpp"

class d3ddevice_proxy;

#define D3D_API __stdcall

class d3dswapchain_proxy : public IDirect3DSwapChain9
{
    std::atomic<ULONG> mRefCount;
    com_ptr<IDirect3DSwapChain9> mSC;

    d3ddevice_proxy* mParent;
    const unsigned mIndex = 0;

    virtual ~d3dswapchain_proxy();
public:
    d3dswapchain_proxy(d3ddevice_proxy* parent,
                       IDirect3DSwapChain9* swapChain,
                       unsigned index);

    HRESULT D3D_API QueryInterface (REFIID riid, void** ppvObj);
    ULONG   D3D_API AddRef(void);
    ULONG   D3D_API Release(void);

    //IDirect3DSwapChain9
    HRESULT D3D_API Present(const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion, DWORD dwFlags);
    HRESULT D3D_API GetFrontBufferData(IDirect3DSurface9 *pDestSurface);
    HRESULT D3D_API GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9 **ppBackBuffer);
    HRESULT D3D_API GetRasterStatus(D3DRASTER_STATUS *pRasterStatus);
    HRESULT D3D_API GetDisplayMode(D3DDISPLAYMODE *pMode);
    HRESULT D3D_API GetDevice(IDirect3DDevice9 **ppDevice);
    HRESULT D3D_API GetPresentParameters(D3DPRESENT_PARAMETERS *pPresentationParameters);

    //IDirect3DSwapChain9
};

#endif // D3DSWAPCHAIN_PROXY_HPP
