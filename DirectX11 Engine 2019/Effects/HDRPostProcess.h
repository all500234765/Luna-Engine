#pragma once

#include "pc.h"
#include "Engine/RenderTarget/RenderTarget.h"
#include "Engine/DirectX/ConstantBuffer.h"
#include "Engine/DirectX/StructuredBuffer.h"
#include "Engine/DirectX/Shader.h"
#include "Engine/Scene/Texture.h"
#include "Engine/Utility/Utils.h"
#include "Engine/Profiler/ScopedRangeProfiler.h"
#include "Engine/States/BlendState.h"
#include "Engine/States/DepthStencilState.h"
#include "Engine/States/RasterState.h"
#include "Engine/States/TopologyState.h"
#include "Engine/Scene/Sampler.h"
#include "Other/DrawCall.h"

// (WIP)
// Eye adaptation
// Bloom
// Depth of Field
// Bokeh
// 

struct DownScaleInst {
    // Res of downscaled target: x - width, y - height
    uint2 _Res; // Backbuffer / 4

    // Total pixels in the downscaled img
    uint _Domain; // Res.x * Res.y

    // Number of groups dispatched on 1st pass
    uint _GroupSize; // Domain / 1024

    // 
    float1 _Adaptation;
    float1 _BloomThreshold;

    float2 _Alignment;
};

struct FinalPassInst {
    // Eye adaptation / HDR
    float1 _MiddleGrey;
    float1 _LumWhiteSqr;

    // Bloom
    float1 _BloomScale;

    float1 _Alignment2;

    // DoF
    // _ProjValues.x = ;
    float2 _ProjectedValues; // _ProjValues.y / _ProjValues.x
    float2 _DoFFarValues;

    // Bokeh
    float1 _ColorScale;
    float1 _RadiusScale;
    float1 _BokehThreshold;

    uint1 _RenderFlags;
};

class HDRPostProcess {
private:
//#pragma pack(push, 1)
    struct BokehBuffer {
        float4 _Color;
        float2 _Position;
        float1 _Radius;
    };
//#pragma pop()

    struct _IndirectArgs {
        D3D11_DRAW_INSTANCED_INDIRECT_ARGS _Args;
    };

    struct _Geometry {
        float1 _AspectRatio; // w / h
        float3 _Padding;
    };

    Shader *shLuminanceDownScale1; // Dispatch(Width * Height / (16 * 1024), 1, 1);
    Shader *shLuminanceDownScale2; // Dispatch(1, 1, 1);
    Shader *shBloom;               // Dispatch(Width * Height / (16 * 1024), 1, 1);
    Shader *shVerticalFilter;      // Dispatch(Width / 16, Height / (16 * (128 - 12)) + 1, 1);
    Shader *shHorizontalFilter;    // Dispatch(ceil(Width / (16 * (128 - 12))), Height / 16, 1);
    Shader *shBloomReveal;         // Dispatch(ceil(Width * Height / 1024.f)
    Shader *shBokeh;               // VS, GS, PS
    Shader *shEmptyShader;         // Nothing is bound here. It clears shader state
    Shader *shHDRTonemap;          // Dispatch(Width, Height)

    ConstantBuffer *cbDownScale;
    ConstantBuffer *cbFinalPass;
    ConstantBuffer *cbGeometry;
    
    StructuredBuffer<float4> *sbILuminance; // Intermidiate luminance
    StructuredBuffer<float4> *sbALuminance; // Average luminance
    StructuredBuffer<float4> *sbPLuminance; // Previous luminance

    StructuredBuffer<_IndirectArgs> *sbBokehIndirect; // For Indirect instanced rendering
    AppendStructuredBuffer<BokehBuffer> *abBokeh;     // Bokeh buffer

    Texture *_HDRDS;   // HDR Downsampled
    Texture *_Bloom;   // Bloom texture
    Texture *_Bloom2;  // Bloom Intermidiate filtered
    Texture *_Blur;    // Blur In / Out
    Texture *_BlurOut; // Blur Intermidiate
    
    Texture *_BokehTex; // Bokeh texture

    BlendState *bsAdditive;
    Sampler *_LinearSampler;

    // Saves last RT's size.
    // So we can apply other effects too
    float fWidth;
    float fHeight;
public:
    HDRPostProcess() {
        // Create resources
        // Eye Adaptation
        cbDownScale = new ConstantBuffer();
        cbDownScale->CreateDefault(sizeof(DownScaleInst));

        uint32_t Width  = 1366 / 4;
        uint32_t Height = 768  / 4;

        DownScaleInst* __q = (DownScaleInst*)cbDownScale->Map();
            __q->_Res        = { Width, Height };
            __q->_Domain     = __q->_Res.x * __q->_Res.y;
            __q->_GroupSize  = __q->_Domain / 1024;
            __q->_Adaptation = 1.f;
        cbDownScale->Unmap();

        cbFinalPass = new ConstantBuffer();
        cbFinalPass->CreateDefault(sizeof(FinalPassInst));

        // 
        cbGeometry = new ConstantBuffer();
        cbGeometry->CreateDefault(sizeof(_Geometry));

        // Shaders
        shLuminanceDownScale1 = new Shader();
        shLuminanceDownScale1->LoadFile("shLumDownscale1CS.cso", Shader::Compute);
        shLuminanceDownScale1->ReleaseBlobs();

        shLuminanceDownScale2 = new Shader();
        shLuminanceDownScale2->LoadFile("shLumDownscale2CS.cso", Shader::Compute);
        shLuminanceDownScale2->ReleaseBlobs();

        shHorizontalFilter = new Shader();
        shHorizontalFilter->LoadFile("shHorizontalFilterCS.cso", Shader::Compute);
        shHorizontalFilter->ReleaseBlobs();

        shVerticalFilter = new Shader();
        shVerticalFilter->LoadFile("shVerticalFilterCS.cso", Shader::Compute);
        shVerticalFilter->ReleaseBlobs();

        shBloomReveal = new Shader();
        shBloomReveal->LoadFile("shBloomRevealCS.cso", Shader::Compute);
        shBloomReveal->ReleaseBlobs();

        shEmptyShader = new Shader();
        shEmptyShader->SetNullShader(Shader::Vertex  );
        shEmptyShader->SetNullShader(Shader::Geometry);
        shEmptyShader->SetNullShader(Shader::Pixel   );
        shEmptyShader->SetNullShader(Shader::Hull    );
        shEmptyShader->SetNullShader(Shader::Domain  );
        shEmptyShader->SetNullShader(Shader::Compute );

        shHDRTonemap = new Shader();
        shHDRTonemap->LoadFile("shHDRTonemap.cso", Shader::Compute);
        shHDRTonemap->ReleaseBlobs();

        shBokeh = new Shader();
        shBokeh->LoadFile("shBokehVS.cso", Shader::Vertex  );
        shBokeh->LoadFile("shBokehGS.cso", Shader::Geometry);
        shBokeh->LoadFile("shBokehPS.cso", Shader::Pixel   );
        shBokeh->ReleaseBlobs();

        shBloom = new Shader();
        shBloom->LoadFile("shBloomCS.cso", Shader::Compute);
        shBloom->ReleaseBlobs();

        // Intermidiate luminance
        sbILuminance = new StructuredBuffer<float4>();
        std::vector<float4> _LumData;
        size_t num = (size_t)ceil(float(Width * Height) / (16.f * 1024.f));
        _LumData.resize(num);

        sbILuminance->CreateDefault((UINT)num, &_LumData[0], true);

        // Average luminance
        sbALuminance = new StructuredBuffer<float4>();
        std::vector<float4> _ALumData;
        _ALumData.resize(1);

        sbALuminance->CreateDefault(1, &_ALumData[0], true);

        // Prev luminance
        sbPLuminance = new StructuredBuffer<float4>();
        std::vector<float4> _PLumData;
        _PLumData.resize(1);

        sbPLuminance->CreateDefault(1, &_PLumData[0], true);

        // Bokeh indirect buffer
        abBokeh = new AppendStructuredBuffer<BokehBuffer>();
        sbBokehIndirect = new StructuredBuffer<_IndirectArgs>();

        _IndirectArgs _Indirect;
        _Indirect._Args.InstanceCount          = 128;
        _Indirect._Args.StartInstanceLocation  = 0;
        _Indirect._Args.StartVertexLocation    = 0;
        _Indirect._Args.VertexCountPerInstance = 1;

        abBokeh->CreateDefault(128, nullptr, true, 0, false);
        sbBokehIndirect->CreateDefault(1, &_Indirect, false, 0, true);

        // Create UAV textures
        DXGI_FORMAT format1 = DXGI_FORMAT_R16G16B16A16_FLOAT, 
                    format2 = DXGI_FORMAT_R32_FLOAT;
        _HDRDS   = new Texture(tf_dim_2 | tf_UAV, format1, Width, Height, 1u, 1u, "HDR Downsample");
        _Bloom   = new Texture(tf_dim_2 | tf_UAV, format1, Width, Height, 1u, 1u, "HDR Bloom");
        _Bloom2  = new Texture(tf_dim_2 | tf_UAV, format1, Width, Height, 1u, 1u, "HDR Bloom 2");
        _Blur    = new Texture(tf_dim_2 | tf_UAV, format1, Width, Height, 1u, 1u, "HDR Blur");
        _BlurOut = new Texture(tf_dim_2 | tf_UAV, format1, Width, Height, 1u, 1u, "HDR Blur output");

        _BokehTex = new Texture("../Textures/Bokeh.dds");
        //_BokehTex = new Texture(); // "../Textures/Bokeh.dds", false, false);
        //_BokehTex->Load("../Textures/Bokeh5.dds", false, false);
        
        // Create blend state
        bsAdditive = new BlendState();
        D3D11_BLEND_DESC pDesc;
        pDesc.AlphaToCoverageEnable          = false;
        pDesc.IndependentBlendEnable         = false;
        pDesc.RenderTarget[0].BlendEnable    = true;
        pDesc.RenderTarget[0].SrcBlend       = D3D11_BLEND_SRC_ALPHA;
        pDesc.RenderTarget[0].DestBlend      = D3D11_BLEND_INV_SRC_ALPHA;
        pDesc.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
        pDesc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_SRC_ALPHA;
        pDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        pDesc.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
        pDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        bsAdditive->Create(pDesc, { 0.f, 0.f, 0.f, 1.f });

        // Create sampler state
        _LinearSampler = new Sampler();
        D3D11_SAMPLER_DESC pSamplerDesc;
        pSamplerDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        pSamplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
        pSamplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
        pSamplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
        pSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        pSamplerDesc.MaxLOD         = D3D11_FLOAT32_MAX;
        pSamplerDesc.MinLOD         = 0;
        pSamplerDesc.MipLODBias     = 0;
        pSamplerDesc.MaxAnisotropy  = 16;

        _LinearSampler->Create(pSamplerDesc);
    };

    ~HDRPostProcess() {
        // Release resources
        shLuminanceDownScale1->Release();
        shLuminanceDownScale2->Release();
        shHorizontalFilter->Release();
        shVerticalFilter->Release();
        shBloomReveal->Release();
        shEmptyShader->Release();
        shHDRTonemap->Release();
        shBokeh->Release();
        shBloom->Release();

        sbILuminance->Release();
        sbALuminance->Release();

        sbBokehIndirect->Release();
        abBokeh->Release();
        
        cbDownScale->Release();
        cbFinalPass->Release();
        cbGeometry->Release();

        _HDRDS->Release();
        _Bloom->Release();
        _Bloom2->Release();
        _Blur->Release();
        _BlurOut->Release();
        _BokehTex->Release();

        bsAdditive->Release();
        _LinearSampler->Release();

        delete bsAdditive;
        delete _LinearSampler;

        delete shLuminanceDownScale1;
        delete shLuminanceDownScale2;
        delete shHorizontalFilter;
        delete shVerticalFilter;
        delete shBloomReveal;
        delete shEmptyShader;
        delete shHDRTonemap;
        delete shBokeh;
        delete shBloom;

        delete sbILuminance;
        delete sbALuminance;

        delete sbBokehIndirect;
        delete abBokeh;

        delete cbDownScale;
        delete cbFinalPass;
        delete cbGeometry;

        delete _HDRDS;
        delete _Bloom;
        delete _Bloom2;
        delete _Blur;
        delete _BlurOut;
        delete _BokehTex;
    };

    // Render Buffer MUST contain
    //  - Diffuse HDR texture in slot 0
    //  - Depth buffer
    template<size_t dim, size_t BufferNum, bool DepthBuffer=false, 
             size_t ArraySize=1,  /* if Cube == true  => specify how many cubemaps 
                                                         to create per RT buffer   */
             bool WillHaveMSAA=false, bool Cube=false>
    void Begin(RenderTarget<dim, BufferNum, DepthBuffer, ArraySize, WillHaveMSAA, Cube> *RB, ID3D11ShaderResourceView* color) {
        ScopedRangeProfiler s1(L"HDR Pass");

        // Avg luminance pass
        // 1st Pass
        RB->Bind(color, Shader::Compute, 0);          // SRV
        sbILuminance->Bind(Shader::Compute, 0, true); // UAV
        _HDRDS->Bind(Shader::Compute, 1, true);       // RWTexture2D; _HDRDS
        cbDownScale->Bind(Shader::Compute, 0);        // CB
        
        fWidth  = (float)RB->GetWidth();
        fHeight = (float)RB->GetHeight();
        UINT X  = (UINT)ceil(fWidth * fHeight / (16.f * 1024.f));
        shLuminanceDownScale1->Dispatch(X, 1, 1);
        
        // Unbind slots
        LunaEngine::CSDiscardUAV<2>();
        LunaEngine::CSDiscardSRV<1>();
        LunaEngine::CSDiscardCB <1>();

        // 2nd Pass
        sbILuminance->Bind(Shader::Compute, 0, true); // UAV
        sbALuminance->Bind(Shader::Compute, 1, true); // UAV
        sbPLuminance->Bind(Shader::Compute, 0);       // Texture / StructuredBuffer
        cbDownScale->Bind(Shader::Compute, 0);        // CB

        shLuminanceDownScale2->Dispatch(1, 1, 1);

        // Unbind slots
        LunaEngine::CSDiscardUAV<2>();
        LunaEngine::CSDiscardSRV<1>();
        LunaEngine::CSDiscardCB <1>();

        // Bloom pass
        _HDRDS->Bind(Shader::Compute, 0);       // Texture2D
        sbALuminance->Bind(Shader::Compute, 1); // SRV
        _Bloom->Bind(Shader::Compute, 0, true); // UAV
        cbDownScale->Bind(Shader::Compute, 0);  // CB

        shBloom->Dispatch(X, 1, 1);

        // Copy _HDRDS to _Blur
        _Blur->Copy(_HDRDS);

        // Unbind slots
        LunaEngine::CSDiscardUAV<1>();
        LunaEngine::CSDiscardSRV<2>();
        LunaEngine::CSDiscardCB <1>();

        //////////////////////////// Bloom Blur ////////////////////////////
        // Horizontal pass
        cbDownScale->Bind(Shader::Compute, 0);   // CB
        _Bloom->Bind(Shader::Compute, 0);        // Texture2D; _Input
        _Bloom2->Bind(Shader::Compute, 0, true); // RWTexture2D; _Output

        shHorizontalFilter->Dispatch((UINT)ceil(fWidth / (4.f * (128.f - 12.f))), (UINT)ceil(fHeight / 4.f), 1);

        // Unbind slots
        LunaEngine::CSDiscardUAV<1>();
        LunaEngine::CSDiscardSRV<1>();
        LunaEngine::CSDiscardCB <1>();

        // Vertical pass
        cbDownScale->Bind(Shader::Compute, 0);  // CB
        _Bloom2->Bind(Shader::Compute, 0);      // Texture2D; _Input
        _Bloom->Bind(Shader::Compute, 0, true); // RWTexture2D; _Output

        shVerticalFilter->Dispatch((UINT)ceil(fWidth / 4.f), (UINT)ceil(fHeight / (4.f * (128.f - 12.f))), 1);

        // Unbind slots
        LunaEngine::CSDiscardUAV<1>();
        LunaEngine::CSDiscardSRV<1>();
        LunaEngine::CSDiscardCB <1>();

        //////////////////////////// Bloom Blur ////////////////////////////
        // Vertical pass
        /*cbDownScale->Bind(Shader::Compute, 0);        // CB
        _Bloom->Bind(Shader::Compute, 0);             // Texture2D; _Input
        _Bloom2->Bind(Shader::Compute, 0, true);      // RWTexture2D; _Output

        shHorizontalFilter->Dispatch((UINT)ceil(RB->GetWidth() / (4.f * (128.f - 12.f))), (UINT)ceil(RB->GetHeight() / 4.f), 1);

        // Unbind slots
        LunaEngine::CSDiscardUAV<2>();
        LunaEngine::CSDiscardSRV<1>();
        LunaEngine::CSDiscardCB <1>();*/

        //////////////////////////// Depth Of Field Blur ////////////////////////////
        // Apply EyeAdaptation to DoF before bluring
        /*_Blur->Bind(Shader::Compute, 0, true);  // RWTexture2D; _Texture; UAV
        sbALuminance->Bind(Shader::Compute, 0); // SRV
        cbDownScale->Bind(Shader::Compute, 0);  // CB
        cbFinalPass->Bind(Shader::Compute, 1);  // CB

        shHDRTonemap->Dispatch((UINT)ceil(fWidth / 4.f), (UINT)ceil(fHeight / 4.f), 1);

        LunaEngine::CSDiscardUAV<1>();
        LunaEngine::CSDiscardSRV<1>();
        LunaEngine::CSDiscardCB <2>();*/

        // Horizontal pass
        cbDownScale->Bind(Shader::Compute, 0);    // CB
        _Blur->Bind(Shader::Compute, 0);          // Texture2D; _Input; SRV
        _BlurOut->Bind(Shader::Compute, 0, true); // RWTexture2D; _Output; UAV

        shVerticalFilter->Dispatch((UINT)ceil(fWidth / 4.f), (UINT)ceil(fHeight / (4.f * (128.f - 12.f))), 1);

        // Unbind slots
        LunaEngine::CSDiscardUAV<1>();
        LunaEngine::CSDiscardSRV<1>();
        LunaEngine::CSDiscardCB <1>();

        // Vertical pass
        cbDownScale->Bind(Shader::Compute, 0); // CB
        _BlurOut->Bind(Shader::Compute, 0);    // Texture2D; _Input
        _Blur->Bind(Shader::Compute, 0, true); // RWTexture2D; _Output

        shHorizontalFilter->Dispatch((UINT)ceil(fWidth / (4.f * (128.f - 12.f))), (UINT)ceil(fHeight / 4.f), 1);

        // Unbind slots
        LunaEngine::CSDiscardUAV<1>();
        LunaEngine::CSDiscardSRV<1>();
        LunaEngine::CSDiscardCB <1>();

        //////////////////////////// Bokeh reveal ////////////////////////////
        RB->Bind(color, Shader::Compute, 0);     // Texture2D; SRV  // Diffuse
        RB->Bind(0u, Shader::Compute, 1);        // Texture2D; SRV  // Depth
        sbALuminance->Bind(Shader::Compute, 2);  // SRV
        cbDownScale->Bind(Shader::Compute, 0);   // CB
        cbFinalPass->Bind(Shader::Compute, 1);   // CB
        abBokeh->Bind(Shader::Compute, 0, true); // UAV

        shBloomReveal->Dispatch((UINT)ceil(fWidth * fHeight / 1024.f), 1, 1);

        // Unbind slots
        LunaEngine::CSDiscardUAV<1>();
        LunaEngine::CSDiscardSRV<3>();
        LunaEngine::CSDiscardCB <2>();
    }

    // On Post-Processing Step use next 2 functions
    void BindFinalPass(Shader::ShaderType shader=Shader::Pixel, UINT slot=0) {
        cbFinalPass->Bind(shader, slot);
    }
    
    void BindLuminance(Shader::ShaderType shader=Shader::Pixel, UINT slot=4) {
        sbALuminance->Bind(shader, slot);
    }

    void BindBloom(Shader::ShaderType shader=Shader::Pixel, UINT slot=5) {
        _Bloom->Bind(shader, slot);
    }

    void BindBlur(Shader::ShaderType shader=Shader::Pixel, UINT slot=6) {
        _Blur->Bind(shader, slot);
    }

    // After post-processing
    void End() {
        ScopedRangeProfiler s1(L"HDR swap");

        // 
        gDirectX->gContext->CopyResource(sbPLuminance->GetBuffer(), sbALuminance->GetBuffer());

        // Update constant buffer
        _Geometry *dbGeometry = (_Geometry*)cbGeometry->Map();
            dbGeometry->_AspectRatio = fWidth / fHeight;
        cbGeometry->Unmap();

        // Copy amount of appended highlights, so we can render them
        //gDirectX->gContext->CopyStructureCount(sbBokehIndirect->GetBuffer(), 0, abBokeh->GetUAV());

        // Update states
        BlendState::Push();
        bsAdditive->Bind();

        shBokeh->Bind();

        gDirectX->gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        gDirectX->gContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
        gDirectX->gContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
        gDirectX->gContext->IASetInputLayout(nullptr);

        // Bind resources
        //_Blur->Bind(Shader::Pixel, 0);
        _BokehTex->Bind(Shader::Pixel, 0);
        _LinearSampler->Bind(Shader::Pixel, 0);
        abBokeh->Bind(Shader::Vertex, 0);
        cbGeometry->Bind(Shader::Geometry, 0);

        // Render bokeh
        DXDrawInstancedIndirect(sbBokehIndirect->GetBuffer(), 0);

        // Restore old states
        shEmptyShader->Bind();
        BlendState::Pop();
        gDirectX->gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        LunaEngine::VSDiscardSRV<1>();
        LunaEngine::GSDiscardCB<1>();
        LunaEngine::PSDiscardSRV<2>();
    }

    // Resize
    void Resize(UINT Width, UINT Height) {
        _HDRDS->Resize(  Width / 4, Height / 4);
        _Bloom->Resize(  Width / 4, Height / 4);
        _Bloom2->Resize( Width / 4, Height / 4);
        _BlurOut->Resize(Width / 4, Height / 4);
        _Blur->Resize(   Width / 4, Height / 4);
    }

    // Update constant buffers
    DownScaleInst* MapDownScale() { return (DownScaleInst*)cbDownScale->Map(); };
    void UnmapDownScale() { cbDownScale->Unmap(); }

    FinalPassInst* MapFinalPass() { return (FinalPassInst*)cbFinalPass->Map(); };
    void UnmapFinalPass() { cbFinalPass->Unmap(); }
};
