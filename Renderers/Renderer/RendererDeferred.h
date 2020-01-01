#pragma once

#include "pc.h"
#include "RendererBase.h"

class RendererDeferred: public RendererBase {
private:
    // Defaults
    struct {
        union {
            float4 black_void1[4];
            const float black_void2[4] = { 0.f, 0.f, 0.f, 0.f };
        };

        union {
            float4 white_snow1[4];
            const float white_snow2[4] = { 1.f, 1.f, 1.f, 1.f };
        };


    } s_clear;

    struct {
        struct {
            Texture *checkboard{};
            Texture *bluenoise_rg_512{};
            Texture *black{};
            Texture *white{};
        } tex{};

        struct {
            SamplerState point{};
            SamplerState linear{};
            SamplerState point_comp{};
            SamplerState linear_comp{};
        } sampl{};

        // TODO: Do clustered rendering
        CubemapTexture *mCubemap{};

    } s_material{};

    struct {
        struct {
            BlendState *normal;
            BlendState *add;
        } blend;

        struct {
            // Depth test/write, no stencil
            DepthStencilState *normal;
            DepthStencilState *norw;
        } depth;

        struct {
            RasterState *normal;
            RasterState *wire;
            RasterState *normal_scissors;
            RasterState *wire_scissors;
        } raster;
    } s_states{};

    // Effects
    HDRPostProcess                 *gHDRPostProcess{};
    SSAOPostProcess                *gSSAOPostProcess{};
    SSLRPostProcess                *gSSLRPostProcess{};
    SSLFPostProcess                *gSSLFPostProcess{};
    CascadeShadowMapping<3, false> *gCascadeShadowMapping{};
    //CoverageBuffer                 *gCoverageBuffer{};
    OrderIndendentTransparency     *gOrderIndendentTransparency{};

    SSLRArgs    gSSLRArgs{};
    SSAOArgs    gSSAOArgs{};
    CSMArgs     gCSMArgs{};
    //CBuffArgs   gCBuffArgs{};
    OITSettings gOITSettings{};

    // Resources
    RenderTarget2DDepthMSAA       *rtDepth{};
    RenderTarget2DColor4DepthMSAA *rtGBuffer{}, *rtCombinedGBuffer{};
    RenderTarget2DColor2DepthMSAA *rtTransparency{};
    RenderTarget2DColor1          *rtFinalPass{};

    Shader *shSurface{}, *shVertexOnly{}, *shGUI{}, *shPostProcess{}, *shCombinationPass{};
    Shader *shSimpleGUI{};

    // Local
    Scene *mScene{};

    // 
    TransformComponent *IdentityTransf;
    ConstantBuffer *cbTransform;

    // Geometry Passes
    void Shadows();             // Done
    void GBuffer();             // Done
    void OIT();                 // Done

    // Occlusion tests
    void CoverageBuffer();

    // Screen-Space Passes
    void Deferred();
    void SSAO();
    void SSLR();
    void SSLF();
    void FSSSSS();
    void Combine();
    void HDR();

    // Final Passes
    void Final();

    void BindOrtho();

public:
    RendererDeferred(): RendererBase() {};
    ~RendererDeferred() {
        printf_s("[~RendererDeferred]\n");
    };

    virtual void Init()        override;
    virtual void Resize()      override;
    virtual void Render()      override;
    virtual void FinalScreen() override;
    virtual void Release()     override;
    virtual void ImGui()       override;
    virtual void ClearMainRT() override;
    virtual void DebugHUD()    override;

};
