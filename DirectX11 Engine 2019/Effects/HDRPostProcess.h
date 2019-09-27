#pragma once

#include "Engine Includes/MainInclude.h"

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
    float1 _MiddleGrey;
    float1 _LumWhiteSqr;
    float1 _BloomScale;
    float1 _Alignment;
};

class HDRPostProcess {
private:
    Shader *shLuminanceDownScale1; // Dispatch(Width * Height / (16 * 1024), 1, 1);
    Shader *shLuminanceDownScale2; // Dispatch(1, 1, 1);
    Shader *shBloom;               // Dispatch(Width * Height / (16 * 1024), 1, 1);
    Shader *shVerticalFilter;      // Dispatch(Width / 16, Height / (16 * (128 - 12)) + 1, 1);
    Shader *shHorizontalFilter;    // Dispatch(ceil(Width / (16 * (128 - 12))), Height / 16, 1);

    ConstantBuffer *cbDownScale;
    ConstantBuffer *cbFinalPass;
    
    StructuredBuffer<float1> *sbILuminance; // Intermidiate luminance
    StructuredBuffer<float1> *sbALuminance; // Average luminance
    StructuredBuffer<float1> *sbPLuminance; // Previous luminance

    Texture *_HDRDS; // HDR Downsampled
    Texture *_Bloom; // Bloom texture
    Texture *_Bloom2; // Bloom filtered

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

        shBloom = new Shader();
        shBloom->LoadFile("shBloomCS.cso", Shader::Compute);
        shBloom->ReleaseBlobs();

        // Intermidiate luminance
        sbILuminance = new StructuredBuffer<float1>();
        std::vector<float1> _LumData;
        int num = ceil(float(Width * Height) / (16.f * 1024.f));
        _LumData.resize(num);

        sbILuminance->CreateDefault(num, &_LumData[0], true);

        // Average luminance
        sbALuminance = new StructuredBuffer<float1>();
        std::vector<float1> _ALumData;
        _ALumData.resize(1);

        sbALuminance->CreateDefault(1, &_ALumData[0], true);

        // Prev luminance
        sbPLuminance = new StructuredBuffer<float1>();
        std::vector<float1> _PLumData;
        _PLumData.resize(1);

        sbPLuminance->CreateDefault(1, &_PLumData[0], true);

        // Create UAV textures
        DXGI_FORMAT format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        _HDRDS  = new Texture(Width, Height, format, true);
        _Bloom  = new Texture(Width, Height, format, true);
        _Bloom2 = new Texture(Width, Height, format, true);
    };

    ~HDRPostProcess() {
        // Release resources
        shLuminanceDownScale1->Release();
        shLuminanceDownScale2->Release();
        shHorizontalFilter->Release();
        shVerticalFilter->Release();
        shBloom->Release();

        sbILuminance->Release();
        sbALuminance->Release();

        cbDownScale->Release();
        cbFinalPass->Release();

        _HDRDS->Release();
        _Bloom->Release();
        _Bloom2->Release();

        delete shLuminanceDownScale1;
        delete shLuminanceDownScale2;
        delete shHorizontalFilter;
        delete shVerticalFilter;
        delete shBloom;

        delete sbILuminance;
        delete sbALuminance;

        delete cbDownScale;
        delete cbFinalPass;

        delete _HDRDS;
        delete _Bloom;
        delete _Bloom2;
    };

    void Begin(RenderBufferBase *RB) {
        // Avg luminance pass
        // 1st Pass
        RB->BindResource(RB->GetColor0(), Shader::Compute, 0); // SRV
        sbILuminance->Bind(Shader::Compute, 0, true);          // UAV
        _HDRDS->Bind(Shader::Compute, 1, true);                // RWTexture2D; _HDRDS
        cbDownScale->Bind(Shader::Compute, 0);                 // CB
        
        float fWidth  = RB->GetWidth();
        float fHeight = RB->GetHeight();
        UINT X = ceil(fWidth * fHeight / (16.f * 1024.f));
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
        _HDRDS->Bind(Shader::Compute, 0);             // Texture2D
        sbALuminance->Bind(Shader::Compute, 1);       // SRV
        _Bloom->Bind(Shader::Compute, 0, true);       // UAV
        cbDownScale->Bind(Shader::Compute, 0);        // CB

        shBloom->Dispatch(X, 1, 1);

        // Unbind slots
        LunaEngine::CSDiscardUAV<2>();
        LunaEngine::CSDiscardSRV<1>();
        LunaEngine::CSDiscardCB <1>();

        // Vertical pass
        cbDownScale->Bind(Shader::Compute, 0);        // CB
        _Bloom->Bind(Shader::Compute, 0);             // Texture2D; _Input
        _Bloom2->Bind(Shader::Compute, 0, true);      // RWTexture2D; _Output

        shHorizontalFilter->Dispatch((UINT)ceil(RB->GetWidth() / (4.f * (128.f - 12.f))), (UINT)ceil(RB->GetHeight() / 4.f), 1);

        // Unbind slots
        LunaEngine::CSDiscardUAV<2>();
        LunaEngine::CSDiscardSRV<1>();
        LunaEngine::CSDiscardCB <1>();

        // Horizontal pass
        cbDownScale->Bind(Shader::Compute, 0);       // CB
        _Bloom2->Bind(Shader::Compute, 0);           // Texture2D; _Input
        _Bloom->Bind(Shader::Compute, 0, true);      // RWTexture2D; _Output

        shVerticalFilter->Dispatch((UINT)ceil(fWidth / 4.f), (UINT)ceil(fHeight / (4.f * (128.f - 12.f))), 1);

        // Unbind slots
        LunaEngine::CSDiscardUAV<2>();
        LunaEngine::CSDiscardSRV<1>();
        LunaEngine::CSDiscardCB <1>();
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

    void End() {
        gDirectX->gContext->CopyResource(sbPLuminance->GetBuffer(), sbALuminance->GetBuffer());
    }

    // Resize
    void Resize(UINT Width, UINT Height) {
        _HDRDS->Resize(Width / 4, Height / 4);
        _Bloom->Resize(Width / 4, Height / 4);
        _Bloom2->Resize(Width / 4, Height / 4);
    }

    // Update constant buffers
    DownScaleInst* MapDownScale() { return (DownScaleInst*)cbDownScale->Map(); };
    void UnmapDownScale() { cbDownScale->Unmap(); }

    FinalPassInst* MapFinalPass() { return (FinalPassInst*)cbFinalPass->Map(); };
    void UnmapFinalPass() { cbFinalPass->Unmap(); }
};
