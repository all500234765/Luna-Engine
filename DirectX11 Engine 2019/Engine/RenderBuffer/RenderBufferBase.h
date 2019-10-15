#pragma once

#include "Engine/DirectX/DirectXChild.h"
#include "Engine/DirectX/Shader.h"
#include "Engine/DirectX/ConstantBuffer.h"
#include "Engine Includes/Types.h"
//#include "HighLevel/DirectX/Utlities.h"
#include "Engine/Materials/Texture.h"

#include <vector>

typedef enum {
    RTV = 1, 
    DSV = 2, 
    Is2D = 4, 
    Is3D = 8, 
    IsCube = 16,  // TODO: Cube texture support
    IsMSAA = 32

} RenderBufferFlags;

struct sRenderBuffer {
    // Target unit
    union {
        ID3D11DepthStencilView *pDSV;
        ID3D11RenderTargetView *pRTV;
    };

    // Texture unit
    union {
        ID3D11Texture2D *pTexture2D;
        ID3D11Texture3D *pTexture3D;
    };

    // Format unit
    union {
        UINT bpp;
        DXGI_FORMAT format;
    };

    // Resource units
    ID3D11UnorderedAccessView *pUAV;
    ID3D11ShaderResourceView  *pSRV;
    char Flags; // Flags

    void Release() {
        if( ((Flags & Is3D) == Is3D) && pTexture3D != nullptr ) { pTexture3D->Release(); }
        if( ((Flags & Is2D) == Is2D) && pTexture2D != nullptr ) { pTexture2D->Release(); }
        
        if( ((Flags & DSV) == DSV) && pDSV != nullptr ) { pDSV->Release(); }
        if( ((Flags & RTV) == RTV) && pRTV != nullptr ) { pRTV->Release(); }

        if( pSRV ) { pSRV->Release(); }
        if( pUAV ) { pUAV->Release(); }
    }
};

class RenderBufferBase: public DirectXChild {
protected:
    int Width, Height, Depth;
    D3D11_VIEWPORT mVP;

    // MSAA
    // 0=Depth, 1=Color0, ..., N=ColorN
    UINT mMSAASamples[9];

    // Is MSAA enabled
    bool mMSAA = false;

    sRenderBuffer* CreateRTV2D(int W, int H       , DXGI_FORMAT format, bool UAV=false, UINT RTIndex=1);
    sRenderBuffer* CreateDSV2D(int W, int H       , UINT bpp=32       , bool UAV=false);
    sRenderBuffer* CreateRTV3D(int W, int H, int D, DXGI_FORMAT format, bool UAV=false);
    sRenderBuffer* CreateDSV3D(int W, int H, int D, UINT bpp=32       , bool UAV=false);

    struct _MSAA_DepthResolve {
        uint2 _Dim;       // Depth buffer dimensions
        uint1 _Samples;   // MSAA Sample count
        uint1 _Alignment; // Empty
    };

    // MSAA Resolve
    static Shader*         g_shMSAADepthResolve;
    static ConstantBuffer* g_MSAAConstantBuffer;
    static Texture*        g_MSAATextureUAV;

    // if( RenderBufferColorN ) then RBSize = N;
    static void MSAAResolveDepth(RenderBufferBase* RB) {
        float Width  = (float)RB->GetWidth();
        float Height = (float)RB->GetHeight();

        // Resolve Depth Buffer MSAA
        // Update constant buffer
        _MSAA_DepthResolve* BuffInst = (_MSAA_DepthResolve*)g_MSAAConstantBuffer->Map();
            BuffInst->_Dim     = { (UINT)Width, (UINT)Height };
            BuffInst->_Samples = RB->GetSampleNum(1);
        g_MSAAConstantBuffer->Unmap();

        // Calculate dispatch dimensions
        UINT X = (UINT)ceil(Width  / 16.f);
        UINT Y = (UINT)ceil(Height / 16.f);

        // Bind resourses
        sRenderBuffer* SRV = RB->GetDepthBF();

        RB->BindResource(RB->GetDepthBF2(), Shader::Compute, 0, true); // UAV
        RB->BindResource(SRV, Shader::Compute, 0);                     // SRV
        g_MSAAConstantBuffer->Bind(Shader::Compute, 0);                // CB

        // Resolve MSAA
        g_shMSAADepthResolve->Dispatch(X, Y, 1);

        // Discard resources
        {
            ID3D11UnorderedAccessView *pEmpty = nullptr;
            gDirectX->gContext->CSSetUnorderedAccessViews(0, 1, &pEmpty, 0);
        }

        {
            ID3D11Buffer *pEmpty = nullptr;
            gDirectX->gContext->CSSetConstantBuffers(0, 1, &pEmpty);
        }

        {
            ID3D11ShaderResourceView *pEmpty = nullptr;
            gDirectX->gContext->CSSetShaderResources(0, 1, &pEmpty);
        }

        // Copy resources from UAV db to DSV db (depth buffer)
        gDirectX->gContext->CopyResource(RB->GetDepthBF1()->pTexture2D, RB->GetDepthBF2()->pTexture2D);
    }

    void MSAACheck(DXGI_FORMAT format, UINT& SampleCount, UINT& Quality) {
        if( mMSAA ) {
            // Check for MSAA Support
            UINT uQuality = 0;
            UINT Samples = D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT;

            // Find max number of samples and quality level
            for( ; Samples > 1; Samples-- ) {
                gDirectX->gDevice->CheckMultisampleQualityLevels(format, Samples, &uQuality);
                if( uQuality > 0 ) { break; }
            }

            // Update
            if( uQuality > 0 ) {
                SampleCount = Samples;
                Quality     = uQuality;

                std::cout << "Found MSAA Quality and Sample Count combination (Samples="
                          << SampleCount << "; Quality=" << Quality << ")" << std::endl;
            } else {
                std::cout << "Failed to find MSAA Quality and Sample Count combination" << std::endl;
            }
        }
    }

    void CreateNonMSAA(DXGI_FORMAT format, sRenderBuffer** sMSAA, UINT RTIndex) {
        // 
        if( mMSAA ) {
            // Disable MSAA to create non-MSAA RTV
            DisableMSAA();

            // Create non-MSAA RTV
            sMSAA[RTIndex] = CreateRTV2D(Width, Height, format, false, RTIndex);

            // Eban MSAA back again
            EnableMSAA();
        }
    }

    void CreateNonMSAA(UINT bpp, sRenderBuffer** sMSAA) {
        // 
        if( mMSAA ) {
            // Disable MSAA to create non-MSAA RTV
            DisableMSAA();

            // Create non-MSAA RTV
            sMSAA[0] = CreateDSV2D(Width, Height, bpp, false);
            sMSAA[1] = CreateDSV2D(Width, Height, bpp, true );

            // Eban MSAA back again
            EnableMSAA();
        }
    }

public:
    inline void EnableMSAA()  { mMSAA = true;  };
    inline void DisableMSAA() { mMSAA = false; };

    static void GlobalInit() {
        g_shMSAADepthResolve = new Shader();
        g_shMSAADepthResolve->LoadFile("shMSAADepthResolveCS.cso", Shader::Compute);
        g_shMSAADepthResolve->ReleaseBlobs();

        g_MSAAConstantBuffer = new ConstantBuffer();
        g_MSAAConstantBuffer->CreateDefault(sizeof(_MSAA_DepthResolve));
    }

    static void GlobalRelease() {
        if( g_shMSAADepthResolve ) {
            g_shMSAADepthResolve->Release();
            delete g_shMSAADepthResolve;
        }

        if( g_MSAAConstantBuffer ) {
            g_MSAAConstantBuffer->Release();
            delete g_MSAAConstantBuffer;
        }
    }

    void BindResource(sRenderBuffer* data, Shader::ShaderType type, UINT slot=0, bool UAV=false);
    void SetSize(int w, int h);
    int GetWidth();
    int GetHeight();
    int GetDepth();
    UINT GetSampleNum(UINT i) const { return mMSAASamples[i]; };

    virtual sRenderBuffer* GetColor0() = 0;   // Returns color buffer 0                        Texture2D
    virtual sRenderBuffer* GetDepthB() = 0;   // Returns non-MSAA depth buffer                 Texture2D
    virtual sRenderBuffer* GetDepthBF() = 0;  // Returns [MSAA] depth buffer                   Texture2D[MS]
    //virtual sRenderBuffer* GetDepthBFA() = 0; // Returns MSAA depth buffer                     Texture2DMS
    virtual sRenderBuffer* GetDepthBF1() = 0; // Returns non-MSAA depth buffer w/o UAV w/  DSV Texture2D
    virtual sRenderBuffer* GetDepthBF2() = 0; // Returns non-MSAA depth buffer w/  UAV w/o DSV Texture2D

    virtual void MSAAResolve() = 0;

    void BindTarget(sRenderBuffer* dRTV, sRenderBuffer* dDSV);
    void BindTarget(sRenderBuffer* data);
    void BindTarget(std::vector<sRenderBuffer*> dRTV, sRenderBuffer* dDSV);
    void BindTarget(std::vector<sRenderBuffer*> dRTV);
    void BindTarget(sRenderBuffer* dRTV, ID3D11DepthStencilView* pDSV);
    void BindTarget(ID3D11RenderTargetView* pRTV, sRenderBuffer* dDSV);
};
