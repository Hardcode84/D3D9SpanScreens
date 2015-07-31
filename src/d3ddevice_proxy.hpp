#ifndef D3DDEVICE_PROXY_HPP
#define D3DDEVICE_PROXY_HPP

#include <d3d9.h>
#include <d3dx9.h>
#include <atomic>
#include <vector>
#include <memory>

#include "tools/common.hpp"
#include "tools/com_ptr.hpp"
#include "tools/handle_ptr.hpp"

#include "d3d_proxy.hpp"
#include "d3dswapchain_proxy.hpp"

class d3ddevice_proxy : public IDirect3DDevice9
{
    std::atomic<ULONG> mRefCount;
    com_ptr<IDirect3DDevice9> mDevice;
    com_ptr<IDirect3DTexture9> mRenderTarget;
    com_ptr<IDirect3DSurface9> mDepthStencil;
    com_ptr<IDirect3DStateBlock9> mQuadState;
    com_ptr<IDirect3DStateBlock9> mTempState;
    com_ptr<d3dswapchain_proxy> mSwapChain;
    std::vector<wnd_handle_ptr<HWND> > mWindows;
    std::vector<int> mMonMapping;

    com_ptr<d3d_proxy> mParent;

    const bool mRestoreState = true;
    int mWidth = 0;
    int mHeight = 0;
    int mRTWidth = 0;
    int mRTHeight = 0;
    D3DFORMAT mBBFormat = D3DFMT_UNKNOWN;
    D3DFORMAT mDSFormat = D3DFMT_UNKNOWN;
    const unsigned mMonCount = 0;
    const bool mIsHorizontal = true;

    bool mIsInsideScene = false;

    d3ddevice_proxy(d3d_proxy* parent);
    virtual ~d3ddevice_proxy();

    struct Vertex
    {
        float x, y, z, rhw;
        D3DCOLOR    color;
        float tu, tv;
    };
    static const DWORD VertexFormat = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;

    bool isInsideScene() const { return mIsInsideScene; }

    HRESULT init();

    HRESULT createRT();
    HRESULT createDS();
    HRESULT createSt(com_ptr<IDirect3DStateBlock9>& stOut) const;
    HRESULT setupQuadState() const;

    HRESULT beginSceneInternal();
    HRESULT endSceneInternal();

    HRESULT onLostInternal();
    HRESULT onResetInternal();

    D3DPRESENT_PARAMETERS mOriginalParams;

    HRESULT createMultimonDevice(IDirect3D9* d3d,
                                 UINT adapter,
                                 D3DDEVTYPE deviceType, HWND hFocusWindow,
                                 DWORD behaviorFlags,
                                 D3DPRESENT_PARAMETERS* pPresentationParameters);

    std::unique_ptr<D3DPRESENT_PARAMETERS[]> createPresParamsArray(D3DPRESENT_PARAMETERS* pPresentationParameters);
public:
    static HRESULT createDeviceProxy(d3d_proxy* parent,
                                     IDirect3D9* d3d,
                                     UINT adapter,
                                     D3DDEVTYPE deviceType, HWND hFocusWindow,
                                     DWORD behaviorFlags,
                                     D3DPRESENT_PARAMETERS* pPresentationParameters,
                                     d3ddevice_proxy** ppRet);

    bool isHorizontal() const { return mIsHorizontal; }

    int getRtWidth()  const { return mRTWidth; }
    int getRtHeight() const { return mRTHeight; }

    HRESULT D3D_API QueryInterface (REFIID riid, void** ppvObj);
    ULONG   D3D_API AddRef(void);
    ULONG   D3D_API Release(void);

    //IDirect3DDevice9
    HRESULT D3D_API TestCooperativeLevel(void);
    UINT    D3D_API GetAvailableTextureMem(void);
    HRESULT D3D_API EvictManagedResources(void);
    HRESULT D3D_API GetDirect3D(IDirect3D9** ppD3D9);
    HRESULT D3D_API GetDeviceCaps(D3DCAPS9* pCaps);
    HRESULT D3D_API GetDisplayMode(UINT iSwapChain,D3DDISPLAYMODE* pMode);
    HRESULT D3D_API GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters);
    HRESULT D3D_API SetCursorProperties(UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap);
    void    D3D_API SetCursorPosition(int X,int Y,DWORD Flags);
    BOOL    D3D_API ShowCursor(BOOL bShow);
    HRESULT D3D_API CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain9** pSwapChain)  ;
    HRESULT D3D_API GetSwapChain(UINT iSwapChain,IDirect3DSwapChain9** pSwapChain);
    UINT    D3D_API GetNumberOfSwapChains(void);
    HRESULT D3D_API Reset(D3DPRESENT_PARAMETERS* pPresentationParameters);
    HRESULT D3D_API Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion);
    HRESULT D3D_API GetBackBuffer(UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer);
    HRESULT D3D_API GetRasterStatus(UINT iSwapChain,D3DRASTER_STATUS* pRasterStatus);
    HRESULT D3D_API SetDialogBoxMode(BOOL bEnableDialogs);
    void    D3D_API SetGammaRamp(UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp);
    void    D3D_API GetGammaRamp(UINT iSwapChain,D3DGAMMARAMP* pRamp);
    HRESULT D3D_API CreateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle);
    HRESULT D3D_API CreateVolumeTexture(UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle);
    HRESULT D3D_API CreateCubeTexture(UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle);
    HRESULT D3D_API CreateVertexBuffer(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle);
    HRESULT D3D_API CreateIndexBuffer(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle);
    HRESULT D3D_API CreateRenderTarget(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle);
    HRESULT D3D_API CreateDepthStencilSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle);
    HRESULT D3D_API UpdateSurface(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint);
    HRESULT D3D_API UpdateTexture(IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture);
    HRESULT D3D_API GetRenderTargetData(IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface);
    HRESULT D3D_API GetFrontBufferData(UINT iSwapChain,IDirect3DSurface9* pDestSurface);
    HRESULT D3D_API StretchRect(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter);
    HRESULT D3D_API ColorFill(IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color);
    HRESULT D3D_API CreateOffscreenPlainSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle);
    HRESULT D3D_API SetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget);
    HRESULT D3D_API GetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget);
    HRESULT D3D_API SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil);
    HRESULT D3D_API GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface);
    HRESULT D3D_API BeginScene(void);
    HRESULT D3D_API EndScene(void);
    HRESULT D3D_API Clear(DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil);
    HRESULT D3D_API SetTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix);
    HRESULT D3D_API GetTransform(D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix);
    HRESULT D3D_API MultiplyTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix);
    HRESULT D3D_API SetViewport(CONST D3DVIEWPORT9* pViewport);
    HRESULT D3D_API GetViewport(D3DVIEWPORT9* pViewport);
    HRESULT D3D_API SetMaterial(CONST D3DMATERIAL9* pMaterial);
    HRESULT D3D_API GetMaterial(D3DMATERIAL9* pMaterial);
    HRESULT D3D_API SetLight(DWORD Index,CONST D3DLIGHT9* pLight);
    HRESULT D3D_API GetLight(DWORD Index,D3DLIGHT9* pLight);
    HRESULT D3D_API LightEnable(DWORD Index,BOOL Enable);
    HRESULT D3D_API GetLightEnable(DWORD Index,BOOL* pEnable);
    HRESULT D3D_API SetClipPlane(DWORD Index,CONST float* pPlane);
    HRESULT D3D_API GetClipPlane(DWORD Index,float* pPlane);
    HRESULT D3D_API SetRenderState(D3DRENDERSTATETYPE State,DWORD Value);
    HRESULT D3D_API GetRenderState(D3DRENDERSTATETYPE State,DWORD* pValue);
    HRESULT D3D_API CreateStateBlock(D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB);
    HRESULT D3D_API BeginStateBlock(void);
    HRESULT D3D_API EndStateBlock(IDirect3DStateBlock9** ppSB);
    HRESULT D3D_API SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus);
    HRESULT D3D_API GetClipStatus(D3DCLIPSTATUS9* pClipStatus);
    HRESULT D3D_API GetTexture(DWORD Stage,IDirect3DBaseTexture9** ppTexture);
    HRESULT D3D_API SetTexture(DWORD Stage,IDirect3DBaseTexture9* pTexture);
    HRESULT D3D_API GetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue);
    HRESULT D3D_API SetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value);
    HRESULT D3D_API GetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue);
    HRESULT D3D_API SetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value);
    HRESULT D3D_API ValidateDevice(DWORD* pNumPasses);
    HRESULT D3D_API SetPaletteEntries(UINT PaletteNumber,CONST PALETTEENTRY* pEntries);
    HRESULT D3D_API GetPaletteEntries(UINT PaletteNumber,PALETTEENTRY* pEntries);
    HRESULT D3D_API SetCurrentTexturePalette(UINT PaletteNumber);
    HRESULT D3D_API GetCurrentTexturePalette(UINT *PaletteNumber);
    HRESULT D3D_API SetScissorRect(CONST RECT* pRect);
    HRESULT D3D_API GetScissorRect( RECT* pRect);
    HRESULT D3D_API SetSoftwareVertexProcessing(BOOL bSoftware);
    BOOL    D3D_API GetSoftwareVertexProcessing(void);
    HRESULT D3D_API SetNPatchMode(float nSegments);
    float   D3D_API GetNPatchMode(void);
    HRESULT D3D_API DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount);
    HRESULT D3D_API DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount);
    HRESULT D3D_API DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride);
    HRESULT D3D_API DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride);
    HRESULT D3D_API ProcessVertices(UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags);
    HRESULT D3D_API CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl);
    HRESULT D3D_API SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl);
    HRESULT D3D_API GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl);
    HRESULT D3D_API SetFVF(DWORD FVF);
    HRESULT D3D_API GetFVF(DWORD* pFVF);
    HRESULT D3D_API CreateVertexShader(CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader);
    HRESULT D3D_API SetVertexShader(IDirect3DVertexShader9* pShader);
    HRESULT D3D_API GetVertexShader(IDirect3DVertexShader9** ppShader);
    HRESULT D3D_API SetVertexShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount);
    HRESULT D3D_API GetVertexShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount);
    HRESULT D3D_API SetVertexShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount);
    HRESULT D3D_API GetVertexShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount);
    HRESULT D3D_API SetVertexShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount);
    HRESULT D3D_API GetVertexShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount);
    HRESULT D3D_API SetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride);
    HRESULT D3D_API GetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* OffsetInBytes,UINT* pStride);
    HRESULT D3D_API SetStreamSourceFreq(UINT StreamNumber,UINT Divider);
    HRESULT D3D_API GetStreamSourceFreq(UINT StreamNumber,UINT* Divider);
    HRESULT D3D_API SetIndices(IDirect3DIndexBuffer9* pIndexData);
    HRESULT D3D_API GetIndices(IDirect3DIndexBuffer9** ppIndexData);
    HRESULT D3D_API CreatePixelShader(CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader);
    HRESULT D3D_API SetPixelShader(IDirect3DPixelShader9* pShader);
    HRESULT D3D_API GetPixelShader(IDirect3DPixelShader9** ppShader);
    HRESULT D3D_API SetPixelShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount);
    HRESULT D3D_API GetPixelShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount);
    HRESULT D3D_API SetPixelShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount);
    HRESULT D3D_API GetPixelShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount);
    HRESULT D3D_API SetPixelShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount);
    HRESULT D3D_API GetPixelShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount);
    HRESULT D3D_API DrawRectPatch(UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo);
    HRESULT D3D_API DrawTriPatch(UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo);
    HRESULT D3D_API DeletePatch(UINT Handle);
    HRESULT D3D_API CreateQuery(D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery);
};

#endif // D3DDEVICE_PROXY_HPP
