#pragma once

#include "Engine Includes/MainInclude.h"

struct SSLFArgs {

};

class SSLFPostProcess {
private:
    const int LensFlareCount = 10;
    struct LensFlare {
        float4 _Color;
        float1 _Offset;
        float1 _Scale;
    };

    BlendState *bsAdditive;
    BlendState *bsNoColor;
    DepthStencilState *dssDepthNoWrite;

    Query *qOcclusion;
    Query *qPredicate;

    Texture *_Corona;
    Texture *_Aperture0;

    StructuredBuffer<LensFlare> *sbLensFlares;

public:
    SSLFPostProcess() {
        // Create structured buffer
        std::vector<LensFlare> lf;
        lf.reserve(LensFlareCount);

        for( int i = 0; i < 4; i++ ) {
            lf[i]._Offset = 0.f;
            lf[i]._Scale = .028f;
            lf[i]._Color = { .2f, .18f, .15f, .25f };
        }

        lf[4]._Offset = .5f;
        lf[4]._Scale = .075f;
        lf[4]._Color = { .2f, .3f, .55f, 1.f };

        lf[5]._Offset = 1.f;
        lf[5]._Scale = .054f;
        lf[5]._Color = { .024f, .2f, .52f, 1.f };

        lf[6]._Offset = 1.35f;
        lf[6]._Scale = .095f;
        lf[6]._Color = { .032f, .1f, .5f, 1.f };

        lf[7]._Offset = .9f;
        lf[7]._Scale = .065f;
        lf[7]._Color = { .13f, .1f, .5f, 1.f };

        lf[8]._Offset = 1.55f;
        lf[8]._Scale = .038f;
        lf[8]._Color = { .16f, .21f, .44f, 1.f };

        lf[9]._Offset = .25f;
        lf[9]._Scale = .1f;
        lf[9]._Color = { .23f, .21f, .44f, .85f };

        sbLensFlares = new StructuredBuffer<LensFlare>();
        sbLensFlares->CreateDefault(LensFlareCount, lf.data(), false);

        // Create querries
        qOcclusion = new Query();
        qOcclusion->Create(D3D11_QUERY_OCCLUSION);

        qPredicate = new Query();
        qPredicate->Create(D3D11_QUERY_OCCLUSION_PREDICATE);

        // Load textures
        _Aperture0 = new Texture();
        _Aperture0->Load("../Textures/Flare.dds", false, false);

        _Corona = new Texture();
        _Corona->Load("../Textures/Corona.dds", false, false);

        // Create additive blend state
        bsAdditive = new BlendState();
        D3D11_BLEND_DESC pDesc;
        pDesc.AlphaToCoverageEnable                 = false;
        pDesc.IndependentBlendEnable                = false;
        pDesc.RenderTarget[0].BlendEnable           = true;
        pDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
        pDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
        pDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
        pDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_SRC_ALPHA;
        pDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_INV_SRC_ALPHA;
        pDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
        pDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        bsAdditive->Create(pDesc, { 0.f, 0.f, 0.f, 1.f });

        // Create no color blend state
        bsNoColor = new BlendState();
        pDesc.RenderTarget[0].BlendEnable           = false;
        pDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_ONE;
        pDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_ONE;
        pDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
        pDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
        pDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ONE;
        pDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
        pDesc.RenderTarget[0].RenderTargetWriteMask = 0;

        // Create depth stencil state
        dssDepthNoWrite = new DepthStencilState();
        D3D11_DEPTH_STENCIL_DESC pDSSDesc;
        const D3D11_DEPTH_STENCILOP_DESC DefaultStencilOP = {
            D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, 
            D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS
        };

        pDSSDesc.DepthEnable      = false;
        pDSSDesc.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ZERO;
        pDSSDesc.DepthFunc        = D3D11_COMPARISON_GREATER;
        pDSSDesc.StencilEnable    = false;
        pDSSDesc.StencilReadMask  = D3D11_DEFAULT_STENCIL_READ_MASK;
        pDSSDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
        pDSSDesc.FrontFace        = DefaultStencilOP;
        pDSSDesc.BackFace         = DefaultStencilOP;

        dssDepthNoWrite->Create(pDSSDesc);

        
    }

    ~SSLFPostProcess() {
        delete sbLensFlares;

        delete qOcclusion;
        delete qPredicate;

        delete _Corona;
        delete _Aperture0;

        delete bsAdditive;
        delete bsNoColor;
        delete dssDepthNoWrite;
    }

    void Begin() {

    }
};
