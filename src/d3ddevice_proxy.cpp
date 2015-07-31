#include "d3ddevice_proxy.hpp"

#include "d3dswapchain_proxy.hpp"
#include "tools/help_funcs.hpp"

d3ddevice_proxy::d3ddevice_proxy(d3d_proxy* parent):
    mRefCount(1),
    mParent(parent, true),
    mRestoreState(parent->restoreStates()),
    mMonCount(parent->getNumMonitors()),
    mIsHorizontal(parent->isHorizontal())
{
    LOG_FUNCTION();
    const auto scrMapping = parent->getScreensMapping();
    for(unsigned i = 0; i < mMonCount; ++i)
    {
        unsigned bbIndex;
        if(i < scrMapping.size())
        {
            bbIndex = help_funcs::bound(0, (int)scrMapping[i], (int)mMonCount - 1);
        }
        else
        {
            bbIndex = i;
        }
        mMonMapping.push_back(bbIndex);
    }
}

d3ddevice_proxy::~d3ddevice_proxy()
{
    LOG_FUNCTION();
}

HRESULT d3ddevice_proxy::createDeviceProxy(d3d_proxy* parent,
                                           IDirect3D9* d3d,
                                           UINT adapter,
                                           D3DDEVTYPE deviceType,
                                           HWND hFocusWindow,
                                           DWORD behaviorFlags,
                                           D3DPRESENT_PARAMETERS* pPresentationParameters,
                                           d3ddevice_proxy** ppRet)
{
    LOG_STATIC_FUNCTION();
    d3ddevice_proxy* ret = new d3ddevice_proxy(parent);

    HRESULT hr;
    if(FAILED(hr = ret->createMultimonDevice(d3d,
                                             adapter,
                                             deviceType,
                                             hFocusWindow,
                                             behaviorFlags,
                                             pPresentationParameters)) || FAILED(hr = ret->init()))
    {
        LOG_ERROR() << "Unable to init d3ddevice_proxy";
        delete ret;
        *ppRet = nullptr;
        return hr;
    }

    *ppRet = ret;
    LOG_INFO() << "Device created";
    WRITE_VAR(ret->mWidth);
    WRITE_VAR(ret->mHeight);
    WRITE_VAR(ret->mBBFormat);
    return S_OK;
}

HRESULT d3ddevice_proxy::init()
{
    auto hr = createRT();
    if(FAILED(hr))
    {
        return hr;
    }
    if(D3DFMT_UNKNOWN != mDSFormat)
    {
        hr = createDS();
        if(FAILED(hr))
        {
            return hr;
        }
    }
    if(mRestoreState)
    {
        hr = createSt(mTempState);
        if(FAILED(hr))
        {
            return hr;
        }
    }
    hr = createSt(mQuadState);
    if(FAILED(hr))
    {
        return hr;
    }
    IDirect3DSurface9* rtSurf = nullptr;
    CHECK_COM_METHOD(mRenderTarget.get(),GetSurfaceLevel,0, &rtSurf);
    const auto rt = make_com_ptr(rtSurf);
    CHECK_COM_METHOD(mDevice,SetRenderTarget,0, rtSurf);
    CHECK_COM_METHOD(mDevice,SetDepthStencilSurface, mDepthStencil.get());
    return hr;
}

HRESULT d3ddevice_proxy::createRT()
{
    mRenderTarget.reset();
    IDirect3DTexture9* rt = nullptr;
    HRESULT hr;
    CHECK_COM_METHOD(mDevice,CreateTexture,
                     mRTWidth,
                     mRTHeight,
                     1,
                     D3DUSAGE_RENDERTARGET,
                     mBBFormat,
                     D3DPOOL_DEFAULT,
                     &rt,
                     nullptr);
    mRenderTarget.reset(rt);
    return hr;
}

HRESULT d3ddevice_proxy::createDS()
{
    mDepthStencil.reset();
    IDirect3DSurface9* ds = nullptr;
    HRESULT hr;
    CHECK_COM_METHOD(mDevice, CreateDepthStencilSurface,
                     mRTWidth,
                     mRTHeight,
                     mDSFormat,
                     D3DMULTISAMPLE_NONE,
                     0,
                     mOriginalParams.Flags & D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL,
                     &ds,
                     nullptr);
    mDepthStencil.reset(ds);
    return hr;
}

HRESULT d3ddevice_proxy::createSt(com_ptr<IDirect3DStateBlock9>& stOut) const
{
    stOut.reset();
    HRESULT hr;
    CHECK_COM_METHOD(mDevice, BeginStateBlock,);
    HRESULT hr1 = setupQuadState();
    IDirect3DStateBlock9* stTemp = nullptr;
    CHECK_COM_METHOD(mDevice, EndStateBlock, &stTemp);
    auto st = make_com_ptr(stTemp);
    if(FAILED(hr1))
    {
        return hr1;
    }
    stOut = st;
    return hr;
}

HRESULT d3ddevice_proxy::setupQuadState() const
{
    HRESULT hr;
    CHECK_COM_METHOD(mDevice, SetRenderState, D3DRS_ZWRITEENABLE, FALSE);
    CHECK_COM_METHOD(mDevice, SetRenderState, D3DRS_ZENABLE, FALSE);
    CHECK_COM_METHOD(mDevice, SetRenderState, D3DRS_ALPHATESTENABLE, FALSE);
    CHECK_COM_METHOD(mDevice, SetRenderState, D3DRS_ALPHABLENDENABLE, FALSE);
    CHECK_COM_METHOD(mDevice, SetRenderState, D3DRS_ZFUNC, D3DCMP_ALWAYS);
    CHECK_COM_METHOD(mDevice, SetRenderState, D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);
    CHECK_COM_METHOD(mDevice, SetRenderState, D3DRS_STENCILENABLE, FALSE);
    CHECK_COM_METHOD(mDevice, SetRenderState, D3DRS_LIGHTING, FALSE);
    CHECK_COM_METHOD(mDevice, SetIndices, nullptr);
    CHECK_COM_METHOD(mDevice, SetVertexShader, nullptr);
    CHECK_COM_METHOD(mDevice, SetPixelShader, nullptr);
    CHECK_COM_METHOD(mDevice, SetStreamSource, 0, nullptr, 0, 0);
    CHECK_COM_METHOD(mDevice, SetFVF, VertexFormat);
    return hr;
}

HRESULT d3ddevice_proxy::beginSceneInternal()
{
    HRESULT hr;
    CHECK_COM_METHOD_DEBUG(mDevice,BeginScene,);
    IDirect3DSurface9* rtSurf = nullptr;
    CHECK_COM_METHOD_DEBUG(mRenderTarget.get(),GetSurfaceLevel,0, &rtSurf);
    const auto rt = make_com_ptr(rtSurf);
    CHECK_COM_METHOD_DEBUG(mDevice,SetRenderTarget,0, rtSurf);
    CHECK_COM_METHOD_DEBUG(mDevice,SetDepthStencilSurface, mDepthStencil.get());
    mIsInsideScene = true;
    return hr;
}

HRESULT d3ddevice_proxy::endSceneInternal()
{
    HRESULT hr;
    IDirect3DSurface9* dsTemp = nullptr;
    D3DVIEWPORT9 oldViewport;
    if(mRestoreState)
    {
        CHECK_COM_METHOD_DEBUG(mTempState, Capture,);
        mDevice->GetDepthStencilSurface(&dsTemp);
        CHECK_COM_METHOD_DEBUG(mDevice,GetViewport,&oldViewport);
    }
    auto ds = make_com_ptr(dsTemp);

    CHECK_COM_METHOD_DEBUG(mQuadState, Apply,);
    MY_ASSERT(mMonMapping.size() == mMonCount);
    for(unsigned i = 0; i < mMonCount; ++i)
    {
        unsigned bbIndex = mMonMapping[i];
        IDirect3DSurface9* bbTemp = nullptr;
        CHECK_COM_METHOD_DEBUG(mDevice, GetBackBuffer,bbIndex, 0, D3DBACKBUFFER_TYPE_MONO, &bbTemp);
        auto bb = make_com_ptr(bbTemp);
        CHECK_COM_METHOD_DEBUG(mDevice, SetRenderTarget,0, bb.get());
        bb.reset();
        CHECK_COM_METHOD_DEBUG(mDevice, SetDepthStencilSurface,nullptr);
        CHECK_COM_METHOD_DEBUG(mDevice, SetTexture, 0, mRenderTarget.get());
        const int indices[] = {0,1,2,2,3,0};
        if(mIsHorizontal)
        {
            const float w  = mWidth;
            const float tw = 1.0f / mMonCount;
            const Vertex vertices[] = {{ 0, float(mHeight), 0.5f, 1.0f, 0xffffffff, i       * tw, 1.0f},
                                       { 0, 0             , 0.5f, 1.0f, 0xffffffff, i       * tw, 0.0f},
                                       { w, 0             , 0.5f, 1.0f, 0xffffffff, (i + 1) * tw, 0.0f},
                                       { w, float(mHeight), 0.5f, 1.0f, 0xffffffff, (i + 1) * tw, 1.0f}};

            CHECK_COM_METHOD_DEBUG(mDevice, DrawIndexedPrimitiveUP,
                            D3DPT_TRIANGLELIST, 0, 4, 2, indices, D3DFMT_INDEX32, vertices, sizeof(Vertex));
        }
        else
        {
            const float h  = mHeight;
            const float th = 1.0f / mMonCount;
            const Vertex vertices[] = {{ 0            , h, 0.5f, 1.0f, 0xffffffff, 0.0f, (i + 1) * th},
                                       { 0            , 0, 0.5f, 1.0f, 0xffffffff, 0.0f, i       * th},
                                       { float(mWidth), 0, 0.5f, 1.0f, 0xffffffff, 1.0f, i       * th},
                                       { float(mWidth), h, 0.5f, 1.0f, 0xffffffff, 1.0f, (i + 1) * th}};

            CHECK_COM_METHOD_DEBUG(mDevice, DrawIndexedPrimitiveUP,
                            D3DPT_TRIANGLELIST, 0, 4, 2, indices, D3DFMT_INDEX32, vertices, sizeof(Vertex));
        }
    }
    if(mRestoreState)
    {
        CHECK_COM_METHOD_DEBUG(mDevice, SetDepthStencilSurface, ds.get());
        CHECK_COM_METHOD_DEBUG(mTempState, Apply,);
        CHECK_COM_METHOD_DEBUG(mDevice, SetViewport, &oldViewport);
    }
    mIsInsideScene = false;
    return mDevice->EndScene();
}

HRESULT d3ddevice_proxy::onLostInternal()
{
    mRenderTarget.reset();
    mDepthStencil.reset();
    mQuadState.reset();
    mTempState.reset();
    return S_OK;
}

HRESULT d3ddevice_proxy::onResetInternal()
{
    return init();
}

HRESULT d3ddevice_proxy::createMultimonDevice(IDirect3D9* d3d,
                                              UINT adapter,
                                              D3DDEVTYPE deviceType,
                                              HWND hFocusWindow,
                                              DWORD behaviorFlags,
                                              D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    LOG_FUNCTION();
    WRITE_VAR(adapter);
    WRITE_VAR(deviceType);
    WRITE_VAR(behaviorFlags);
    WRITE_FLAG(behaviorFlags, D3DCREATE_ADAPTERGROUP_DEVICE);
    WRITE_FLAG(behaviorFlags, D3DCREATE_DISABLE_DRIVER_MANAGEMENT);
    WRITE_FLAG(behaviorFlags, D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX);
    WRITE_FLAG(behaviorFlags, D3DCREATE_DISABLE_PRINTSCREEN);
    WRITE_FLAG(behaviorFlags, D3DCREATE_DISABLE_PSGP_THREADING);
    WRITE_FLAG(behaviorFlags, D3DCREATE_ENABLE_PRESENTSTATS);
    WRITE_FLAG(behaviorFlags, D3DCREATE_FPU_PRESERVE);
    WRITE_FLAG(behaviorFlags, D3DCREATE_HARDWARE_VERTEXPROCESSING);
    WRITE_FLAG(behaviorFlags, D3DCREATE_MIXED_VERTEXPROCESSING);
    WRITE_FLAG(behaviorFlags, D3DCREATE_MULTITHREADED);
    WRITE_FLAG(behaviorFlags, D3DCREATE_NOWINDOWCHANGES);
    WRITE_FLAG(behaviorFlags, D3DCREATE_PUREDEVICE);
    WRITE_FLAG(behaviorFlags, D3DCREATE_SCREENSAVER);
    WRITE_FLAG(behaviorFlags, D3DCREATE_SOFTWARE_VERTEXPROCESSING);
    CHECK_PARAM(nullptr != hFocusWindow);
    CHECK_PARAM(nullptr != pPresentationParameters);
    WRITE_VAR(pPresentationParameters->BackBufferCount);
    WRITE_VAR(pPresentationParameters->BackBufferFormat);
    WRITE_VAR(pPresentationParameters->BackBufferWidth);
    WRITE_VAR(pPresentationParameters->BackBufferHeight);
    WRITE_VAR(pPresentationParameters->EnableAutoDepthStencil);
    CHECK_PARAM(!pPresentationParameters->Windowed);
    mWindows.clear();
    const int ClassNameLength = 512;
    WCHAR wndClassName[ClassNameLength + 1];
    if(!::GetClassNameW(hFocusWindow, wndClassName, ClassNameLength))
    {
        LOG_ERROR() << "Unable to get window class name: " << getWinError();
        return HRESULT_FROM_WIN32(GetLastError());
    }
    const auto wndStyle =   ::GetWindowLongPtr(hFocusWindow, GWL_STYLE);
    const auto wndExStyle = ::GetWindowLongPtr(hFocusWindow, GWL_EXSTYLE);
    mBBFormat = pPresentationParameters->BackBufferFormat;
    mRTWidth = pPresentationParameters->BackBufferWidth;
    mRTHeight = pPresentationParameters->BackBufferHeight;
    if(isHorizontal())
    {
        mWidth  = pPresentationParameters->BackBufferWidth / mMonCount;
        mHeight = pPresentationParameters->BackBufferHeight;
    }
    else
    {
        mWidth  = pPresentationParameters->BackBufferWidth;
        mHeight = pPresentationParameters->BackBufferHeight / mMonCount;
    }
    for(unsigned i = 1; i < mMonCount; ++i)
    {
        auto wnd = ::CreateWindowExW(wndExStyle,
                                     wndClassName,
                                     L"d3d_proxy window",
                                     wndStyle,
                                     0, 0, mWidth, mHeight, nullptr, nullptr, nullptr, nullptr);
        if(nullptr == wnd)
        {
            LOG_ERROR() << "Unable to create window: " << getWinError();
            const auto ret = GetLastError();
            mWindows.clear();
            return HRESULT_FROM_WIN32(ret);
        }
        mWindows.push_back(wnd);
    }

    if(pPresentationParameters->EnableAutoDepthStencil)
    {
        mDSFormat = pPresentationParameters->AutoDepthStencilFormat;
    }
    else
    {
        mDSFormat = D3DFMT_UNKNOWN;
    }
    const auto params = createPresParamsArray(pPresentationParameters);
    const DWORD flags = behaviorFlags | D3DCREATE_ADAPTERGROUP_DEVICE;
    IDirect3DDevice9* dev = nullptr;
    auto hr = d3d->CreateDevice(adapter, deviceType, hFocusWindow, flags, params.get(), &dev);
    if(FAILED(hr))
    {
        LOG_ERROR() << "Unable to create device: " << getWinErrorFromHr(hr);
        mWindows.clear();
        return hr;
    }
    mDevice.reset(dev);
    mOriginalParams = *pPresentationParameters;
    return hr;
}

std::unique_ptr<D3DPRESENT_PARAMETERS[]>
d3ddevice_proxy::createPresParamsArray(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    LOG_FUNCTION();
    std::unique_ptr<D3DPRESENT_PARAMETERS[]> params(new D3DPRESENT_PARAMETERS[mMonCount]);
    std::fill_n(params.get(), mMonCount, *pPresentationParameters);
    for(unsigned i = 0; i < mMonCount; ++i)
    {
        params[i].BackBufferWidth  = mWidth;
        params[i].BackBufferHeight = mHeight;
        params[i].EnableAutoDepthStencil = FALSE;
        params[i].AutoDepthStencilFormat = D3DFMT_UNKNOWN;
        params[i].Flags &= ~D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
        params[i].Flags &= ~D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
        if(0 != i)
        {
            params[i].hDeviceWindow = mWindows[i - 1].get();
        }
    }
    return params;
}

HRESULT D3D_API d3ddevice_proxy::QueryInterface(const IID &riid, LPVOID *ppvObj)
{
    //LOG_FUNCTION();
    return E_NOINTERFACE;
    //return mDevice->QueryInterface(riid,ppvObj);
}

ULONG   D3D_API d3ddevice_proxy::AddRef()
{
    return ++mRefCount;
}

ULONG   D3D_API d3ddevice_proxy::Release()
{
    const ULONG ret = --mRefCount;
    if(0 == ret)
    {
        delete this;
        return 0;
    }
    return ret;
}


HRESULT d3ddevice_proxy::TestCooperativeLevel(void)
{
    return mDevice->TestCooperativeLevel();
}

UINT d3ddevice_proxy::GetAvailableTextureMem(void)
{
    return mDevice->GetAvailableTextureMem();
}

HRESULT d3ddevice_proxy::EvictManagedResources(void)
{
    return mDevice->EvictManagedResources();
}

HRESULT d3ddevice_proxy::GetDirect3D(IDirect3D9** ppD3D9)
{
    //LOG_FUNCTION();
    CHECK_PARAM(nullptr != ppD3D9);
    mParent->AddRef();
    *ppD3D9 = mParent.get();
    return S_OK;
}

HRESULT d3ddevice_proxy::GetDeviceCaps(D3DCAPS9* pCaps)
{
    LOG_FUNCTION();
    HRESULT hr;
    CHECK_COM_METHOD(mDevice,GetDeviceCaps,pCaps);
    pCaps->AdapterOrdinalInGroup = 0;
    pCaps->NumberOfAdaptersInGroup = 1;
    return hr;
}

HRESULT d3ddevice_proxy::GetDisplayMode(UINT iSwapChain,D3DDISPLAYMODE* pMode)
{
    CHECK_PARAM(0 == iSwapChain);
    HRESULT hr;
    CHECK_COM_METHOD_DEBUG(mDevice,GetDisplayMode,iSwapChain, pMode);
    pMode->Width  = mRTWidth;
    pMode->Height = mRTHeight;
    return hr;
}

HRESULT d3ddevice_proxy::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
    return mDevice->GetCreationParameters(pParameters);
}

HRESULT d3ddevice_proxy::SetCursorProperties(UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap)
{
    return mDevice->SetCursorProperties(XHotSpot,YHotSpot,pCursorBitmap);
}

void    d3ddevice_proxy::SetCursorPosition(int X,int Y,DWORD Flags)
{
    return mDevice->SetCursorPosition(X,Y,Flags);
}

BOOL    d3ddevice_proxy::ShowCursor(BOOL bShow)
{
    return mDevice->ShowCursor(bShow);
}

HRESULT d3ddevice_proxy::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain9** pSwapChain)
{
    return mDevice->CreateAdditionalSwapChain(pPresentationParameters,pSwapChain);
}

HRESULT d3ddevice_proxy::GetSwapChain(UINT iSwapChain,IDirect3DSwapChain9** pSwapChain)
{
    LOG_FUNCTION();
    CHECK_PARAM(0 == iSwapChain);
    HRESULT hr = S_OK;
    if(!mSwapChain)
    {
        IDirect3DSwapChain9* sc = nullptr;
        CHECK_COM_METHOD(mDevice, GetSwapChain, iSwapChain, &sc);
        mSwapChain.reset(new d3dswapchain_proxy(this, sc, iSwapChain));
    }
    mSwapChain->AddRef();
    *pSwapChain = mSwapChain.get();
    return hr;
}

UINT    d3ddevice_proxy::GetNumberOfSwapChains(void)
{
    LOG_FUNCTION();
    return 1;
    //return mDevice->GetNumberOfSwapChains();
}

HRESULT d3ddevice_proxy::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    LOG_FUNCTION();
    CHECK_PARAM(nullptr != pPresentationParameters);
    mIsInsideScene = false;
    HRESULT hr;
    CHECK_STAIC_COM_METHOD(onLostInternal,);
    const auto params = createPresParamsArray(pPresentationParameters);
    CHECK_COM_METHOD(mDevice,Reset,params.get());
    CHECK_STAIC_COM_METHOD(onResetInternal,);
    return hr;
}

HRESULT d3ddevice_proxy::Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
{
    return mDevice->Present( pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT d3ddevice_proxy::GetBackBuffer(UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer)
{
    CHECK_PARAM(0 == iSwapChain);
    CHECK_PARAM(0 == iBackBuffer);
    return mRenderTarget->GetSurfaceLevel(0, ppBackBuffer);
}

HRESULT d3ddevice_proxy::GetRasterStatus(UINT iSwapChain,D3DRASTER_STATUS* pRasterStatus)
{
    return mDevice->GetRasterStatus(iSwapChain,pRasterStatus);
}

HRESULT d3ddevice_proxy::SetDialogBoxMode(BOOL bEnableDialogs)
{
    return mDevice->SetDialogBoxMode(bEnableDialogs);
}

void    d3ddevice_proxy::SetGammaRamp(UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp)
{
    return mDevice->SetGammaRamp(iSwapChain,Flags,pRamp);
}

void    d3ddevice_proxy::GetGammaRamp(UINT iSwapChain,D3DGAMMARAMP* pRamp)
{
    return mDevice->GetGammaRamp(iSwapChain,pRamp);
}

HRESULT d3ddevice_proxy::CreateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle)
{
    return mDevice->CreateTexture(Width,Height,Levels,Usage,Format,Pool,ppTexture,pSharedHandle);
}

HRESULT d3ddevice_proxy::CreateVolumeTexture(UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle)
{
    return mDevice->CreateVolumeTexture(Width,Height,Depth,Levels,Usage,Format,Pool,ppVolumeTexture,pSharedHandle);
}

HRESULT d3ddevice_proxy::CreateCubeTexture(UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle)
{
    return mDevice->CreateCubeTexture(EdgeLength,Levels,Usage,Format,Pool,ppCubeTexture,pSharedHandle);
}

HRESULT d3ddevice_proxy::CreateVertexBuffer(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle)
{
    return mDevice->CreateVertexBuffer(Length,Usage,FVF,Pool,ppVertexBuffer,pSharedHandle);
}

HRESULT d3ddevice_proxy::CreateIndexBuffer(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle)
{
    return mDevice->CreateIndexBuffer(Length,Usage,Format,Pool,ppIndexBuffer,pSharedHandle);
}

HRESULT d3ddevice_proxy::CreateRenderTarget(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
    return mDevice->CreateRenderTarget(Width,Height,Format,MultiSample,MultisampleQuality,Lockable,ppSurface,pSharedHandle);
}

HRESULT d3ddevice_proxy::CreateDepthStencilSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
    return mDevice->CreateDepthStencilSurface(Width,Height,Format,MultiSample,MultisampleQuality,Discard,ppSurface,pSharedHandle);
}

HRESULT d3ddevice_proxy::UpdateSurface(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint)
{
    return mDevice->UpdateSurface(pSourceSurface,pSourceRect,pDestinationSurface,pDestPoint);
}

HRESULT d3ddevice_proxy::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture)
{
    return mDevice->UpdateTexture(pSourceTexture,pDestinationTexture);
}

HRESULT d3ddevice_proxy::GetRenderTargetData(IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface)
{
    return mDevice->GetRenderTargetData(pRenderTarget,pDestSurface);
}

HRESULT d3ddevice_proxy::GetFrontBufferData(UINT iSwapChain,IDirect3DSurface9* pDestSurface)
{
    return mDevice->GetFrontBufferData(iSwapChain,pDestSurface);
}

HRESULT d3ddevice_proxy::StretchRect(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter)
{
    return mDevice->StretchRect(pSourceSurface,pSourceRect,pDestSurface,pDestRect,Filter);
}

HRESULT d3ddevice_proxy::ColorFill(IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color)
{
    return mDevice->ColorFill(pSurface,pRect,color);
}

HRESULT d3ddevice_proxy::CreateOffscreenPlainSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
    return mDevice->CreateOffscreenPlainSurface(Width,Height,Format,Pool,ppSurface,pSharedHandle);
}

HRESULT d3ddevice_proxy::SetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget)
{
    return mDevice->SetRenderTarget(RenderTargetIndex,pRenderTarget);
}

HRESULT d3ddevice_proxy::GetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget)
{
    return mDevice->GetRenderTarget(RenderTargetIndex,ppRenderTarget);
}

HRESULT d3ddevice_proxy::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{
    return mDevice->SetDepthStencilSurface(pNewZStencil);
}

HRESULT d3ddevice_proxy::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{
    LOG_FUNCTION();
    return mDevice->GetDepthStencilSurface(ppZStencilSurface);
}

HRESULT d3ddevice_proxy::BeginScene(void)
{
    LOG_FUNCTION();
    return beginSceneInternal();
}

HRESULT d3ddevice_proxy::EndScene(void)
{
    LOG_FUNCTION();
    return endSceneInternal();
}

HRESULT d3ddevice_proxy::Clear(DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil)
{
    return mDevice->Clear(Count,pRects,Flags,Color,Z,Stencil);
}

HRESULT d3ddevice_proxy::SetTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
{
    return mDevice->SetTransform(State,pMatrix);
}

HRESULT d3ddevice_proxy::GetTransform(D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix)
{
    return mDevice->GetTransform(State,pMatrix);
}

HRESULT d3ddevice_proxy::MultiplyTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
{
    return mDevice->MultiplyTransform(State,pMatrix);
}

HRESULT d3ddevice_proxy::SetViewport(CONST D3DVIEWPORT9* pViewport)
{
    /*
    LOG_FUNCTION();
    WRITE_VAR(pViewport->X);
    WRITE_VAR(pViewport->Y);
    WRITE_VAR(pViewport->Width);
    WRITE_VAR(pViewport->Height);
    WRITE_VAR(pViewport->MinZ);
    WRITE_VAR(pViewport->MaxZ);
    */
    return mDevice->SetViewport(pViewport);
}

HRESULT d3ddevice_proxy::GetViewport(D3DVIEWPORT9* pViewport)
{
    return mDevice->GetViewport(pViewport);
}

HRESULT d3ddevice_proxy::SetMaterial(CONST D3DMATERIAL9* pMaterial)
{
    return mDevice->SetMaterial(pMaterial);
}

HRESULT d3ddevice_proxy::GetMaterial(D3DMATERIAL9* pMaterial)
{
    return mDevice->GetMaterial(pMaterial);
}

HRESULT d3ddevice_proxy::SetLight(DWORD Index,CONST D3DLIGHT9* pLight)
{
    return mDevice->SetLight(Index,pLight);
}

HRESULT d3ddevice_proxy::GetLight(DWORD Index,D3DLIGHT9* pLight)
{
    return mDevice->GetLight(Index,pLight);
}

HRESULT d3ddevice_proxy::LightEnable(DWORD Index,BOOL Enable)
{
    return mDevice->LightEnable(Index,Enable);
}

HRESULT d3ddevice_proxy::GetLightEnable(DWORD Index,BOOL* pEnable)
{
    return mDevice->GetLightEnable(Index, pEnable);
}

HRESULT d3ddevice_proxy::SetClipPlane(DWORD Index,CONST float* pPlane)
{
    return mDevice->SetClipPlane(Index, pPlane);
}

HRESULT d3ddevice_proxy::GetClipPlane(DWORD Index,float* pPlane)
{
    return mDevice->GetClipPlane(Index,pPlane);
}

HRESULT d3ddevice_proxy::SetRenderState(D3DRENDERSTATETYPE State,DWORD Value)
{
    return mDevice->SetRenderState(State, Value);
}

HRESULT d3ddevice_proxy::GetRenderState(D3DRENDERSTATETYPE State,DWORD* pValue)
{
    return mDevice->GetRenderState(State, pValue);
}

HRESULT d3ddevice_proxy::CreateStateBlock(D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB)
{
    return mDevice->CreateStateBlock(Type,ppSB);
}

HRESULT d3ddevice_proxy::BeginStateBlock(void)
{
    return mDevice->BeginStateBlock();
}

HRESULT d3ddevice_proxy::EndStateBlock(IDirect3DStateBlock9** ppSB)
{
    return mDevice->EndStateBlock(ppSB);
}

HRESULT d3ddevice_proxy::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus)
{
    return mDevice->SetClipStatus(pClipStatus);
}

HRESULT d3ddevice_proxy::GetClipStatus(D3DCLIPSTATUS9* pClipStatus)
{
    return mDevice->GetClipStatus( pClipStatus);
}

HRESULT d3ddevice_proxy::GetTexture(DWORD Stage,IDirect3DBaseTexture9** ppTexture)
{
    return mDevice->GetTexture(Stage,ppTexture);
}

HRESULT d3ddevice_proxy::SetTexture(DWORD Stage,IDirect3DBaseTexture9* pTexture)
{
    return mDevice->SetTexture(Stage,pTexture);
}

HRESULT d3ddevice_proxy::GetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue)
{
    return mDevice->GetTextureStageState(Stage,Type, pValue);
}

HRESULT d3ddevice_proxy::SetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value)
{
    return mDevice->SetTextureStageState(Stage,Type,Value);
}

HRESULT d3ddevice_proxy::GetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue)
{
    return mDevice->GetSamplerState(Sampler,Type, pValue);
}

HRESULT d3ddevice_proxy::SetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value)
{
    return mDevice->SetSamplerState(Sampler,Type,Value);
}

HRESULT d3ddevice_proxy::ValidateDevice(DWORD* pNumPasses)
{
    return mDevice->ValidateDevice( pNumPasses);
}

HRESULT d3ddevice_proxy::SetPaletteEntries(UINT PaletteNumber,CONST PALETTEENTRY* pEntries)
{
    return mDevice->SetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT d3ddevice_proxy::GetPaletteEntries(UINT PaletteNumber,PALETTEENTRY* pEntries)
{
    return mDevice->GetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT d3ddevice_proxy::SetCurrentTexturePalette(UINT PaletteNumber)
{
    return mDevice->SetCurrentTexturePalette(PaletteNumber);
}

HRESULT d3ddevice_proxy::GetCurrentTexturePalette(UINT *PaletteNumber)
{
    return mDevice->GetCurrentTexturePalette(PaletteNumber);
}

HRESULT d3ddevice_proxy::SetScissorRect(CONST RECT* pRect)
{
    return mDevice->SetScissorRect( pRect);
}

HRESULT d3ddevice_proxy::GetScissorRect( RECT* pRect)
{
    return mDevice->GetScissorRect( pRect);
}

HRESULT d3ddevice_proxy::SetSoftwareVertexProcessing(BOOL bSoftware)
{
    return mDevice->SetSoftwareVertexProcessing(bSoftware);
}

BOOL    d3ddevice_proxy::GetSoftwareVertexProcessing(void)
{
    return mDevice->GetSoftwareVertexProcessing();
}

HRESULT d3ddevice_proxy::SetNPatchMode(float nSegments)
{
    return mDevice->SetNPatchMode(nSegments);
}

float   d3ddevice_proxy::GetNPatchMode(void)
{
    return mDevice->GetNPatchMode();
}

HRESULT d3ddevice_proxy::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount)
{
    return mDevice->DrawPrimitive(PrimitiveType,StartVertex,PrimitiveCount);
}

HRESULT d3ddevice_proxy::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount)
{
    return mDevice->DrawIndexedPrimitive(PrimitiveType,BaseVertexIndex,MinVertexIndex,NumVertices,startIndex,primCount);
}

HRESULT d3ddevice_proxy::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
    return mDevice->DrawPrimitiveUP(PrimitiveType,PrimitiveCount,pVertexStreamZeroData,VertexStreamZeroStride);
}

HRESULT d3ddevice_proxy::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
    return mDevice->DrawIndexedPrimitiveUP(PrimitiveType,MinVertexIndex,NumVertices,PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData,VertexStreamZeroStride);
}

HRESULT d3ddevice_proxy::ProcessVertices(UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags)
{
    return mDevice->ProcessVertices( SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
}

HRESULT d3ddevice_proxy::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl)
{
    return mDevice->CreateVertexDeclaration( pVertexElements,ppDecl);
}

HRESULT d3ddevice_proxy::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
    return mDevice->SetVertexDeclaration(pDecl);
}

HRESULT d3ddevice_proxy::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{
    return mDevice->GetVertexDeclaration(ppDecl);
}

HRESULT d3ddevice_proxy::SetFVF(DWORD FVF)
{
    return mDevice->SetFVF(FVF);
}

HRESULT d3ddevice_proxy::GetFVF(DWORD* pFVF)
{
    return mDevice->GetFVF(pFVF);
}

HRESULT d3ddevice_proxy::CreateVertexShader(CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader)
{
    return mDevice->CreateVertexShader(pFunction,ppShader);
}

HRESULT d3ddevice_proxy::SetVertexShader(IDirect3DVertexShader9* pShader)
{
    return mDevice->SetVertexShader(pShader);
}

HRESULT d3ddevice_proxy::GetVertexShader(IDirect3DVertexShader9** ppShader)
{
    return mDevice->GetVertexShader(ppShader);
}

HRESULT d3ddevice_proxy::SetVertexShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
    return mDevice->SetVertexShaderConstantF(StartRegister,pConstantData, Vector4fCount);
}

HRESULT d3ddevice_proxy::GetVertexShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{
    return mDevice->GetVertexShaderConstantF(StartRegister,pConstantData,Vector4fCount);
}

HRESULT d3ddevice_proxy::SetVertexShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{
    return mDevice->SetVertexShaderConstantI(StartRegister,pConstantData,Vector4iCount);
}

HRESULT d3ddevice_proxy::GetVertexShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{
    return mDevice->GetVertexShaderConstantI(StartRegister,pConstantData,Vector4iCount);
}

HRESULT d3ddevice_proxy::SetVertexShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{
    return mDevice->SetVertexShaderConstantB(StartRegister,pConstantData,BoolCount);
}

HRESULT d3ddevice_proxy::GetVertexShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{
    return mDevice->GetVertexShaderConstantB(StartRegister,pConstantData,BoolCount);
}

HRESULT d3ddevice_proxy::SetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride)
{
    return mDevice->SetStreamSource(StreamNumber,pStreamData,OffsetInBytes,Stride);
}

HRESULT d3ddevice_proxy::GetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* OffsetInBytes,UINT* pStride)
{
    return mDevice->GetStreamSource(StreamNumber,ppStreamData,OffsetInBytes,pStride);
}

HRESULT d3ddevice_proxy::SetStreamSourceFreq(UINT StreamNumber,UINT Divider)
{
    return mDevice->SetStreamSourceFreq(StreamNumber,Divider);
}

HRESULT d3ddevice_proxy::GetStreamSourceFreq(UINT StreamNumber,UINT* Divider)
{
    return mDevice->GetStreamSourceFreq(StreamNumber,Divider);
}

HRESULT d3ddevice_proxy::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
    return mDevice->SetIndices(pIndexData);
}

HRESULT d3ddevice_proxy::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{
    return mDevice->GetIndices(ppIndexData);
}

HRESULT d3ddevice_proxy::CreatePixelShader(CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader)
{
    return mDevice->CreatePixelShader(pFunction,ppShader);
}

HRESULT d3ddevice_proxy::SetPixelShader(IDirect3DPixelShader9* pShader)
{
    return mDevice->SetPixelShader(pShader);
}

HRESULT d3ddevice_proxy::GetPixelShader(IDirect3DPixelShader9** ppShader)
{
    return mDevice->GetPixelShader(ppShader);
}

HRESULT d3ddevice_proxy::SetPixelShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
    return mDevice->SetPixelShaderConstantF(StartRegister,pConstantData,Vector4fCount);
}

HRESULT d3ddevice_proxy::GetPixelShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{
    return mDevice->GetPixelShaderConstantF(StartRegister,pConstantData,Vector4fCount);
}

HRESULT d3ddevice_proxy::SetPixelShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{
    return mDevice->SetPixelShaderConstantI(StartRegister,pConstantData,Vector4iCount);
}

HRESULT d3ddevice_proxy::GetPixelShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{
    return mDevice->GetPixelShaderConstantI(StartRegister,pConstantData,Vector4iCount);
}

HRESULT d3ddevice_proxy::SetPixelShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{
    return mDevice->SetPixelShaderConstantB(StartRegister,pConstantData,BoolCount);
}

HRESULT d3ddevice_proxy::GetPixelShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{
    return mDevice->GetPixelShaderConstantB(StartRegister,pConstantData,BoolCount);
}

HRESULT d3ddevice_proxy::DrawRectPatch(UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{
    return mDevice->DrawRectPatch(Handle,pNumSegs, pRectPatchInfo);
}

HRESULT d3ddevice_proxy::DrawTriPatch(UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{
    return mDevice->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}

HRESULT d3ddevice_proxy::DeletePatch(UINT Handle)
{
    return mDevice->DeletePatch(Handle);
}

HRESULT d3ddevice_proxy::CreateQuery(D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery)
{
    return mDevice->CreateQuery(Type,ppQuery);
}
