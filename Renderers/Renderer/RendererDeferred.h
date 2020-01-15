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
        union {
            Texture* texture_list[8];

            struct {
                Texture *checkboard{};
                Texture *tile_normal{};
                Texture *bluenoise_rg_512{};
                Texture *black{};
                Texture *white{};
                Texture *mgray{}; // Middle gray; 127
                Texture *dgray{}; // Dark gray; 63
                Texture *lgray{}; // Light gray; 190
            } tex{};
        } ti{};

        struct {
            SamplerState point{};
            SamplerState linear{};
            SamplerState point_comp{};
            SamplerState linear_comp{};
        } sampl{};

        // TODO: Do clustered rendering
        Texture *mCubemap{};

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

    // Deferred renderer
    ConstantBuffer *cbDeferredGlobal;
    struct DeferredGlobal {
        #include "Deferred/Global.h"
    };

    ConstantBuffer *cbDeferredLight;
    struct DeferredLight {
        #include "Deferred/Light.h"
    };

    CounterStructuredBuffer<DeferredLight> *sbDeferredLight;

    // Voolumetric lights
    ConstantBuffer *cbVolumetricSettings;
    struct VolumetricSettings {
        //                              fQ = fFar / (fNear - fFar);
        float4 _ProjValues;  // fNear * fQ, fQ, 1 / m[0][0], 1 / m[1][1] // Player
        float4 _ProjValues2; //                                          // Light
        float _GScattering;  // [-1; 1]
        float2 _Scaling;     // Width, Height / Downscaling Factor
        float _MaxDistance;  // 0 - Light Far?
    };

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
    VolumetricSettings gVolumetricSettings{};

    // Resources
    RenderTarget2DDepthMSAA       *rtDepth{};
    RenderTarget2DColor4DepthMSAA *rtGBuffer{}, *rtCombinedGBuffer{};
    RenderTarget2DColor4DepthMSAA *rtTransparency{};
    RenderTarget2DColor1          *rtFinalPass{};
    RenderTarget2DColor2MSAA      *rtDeferred{};

    Texture *mVolumetricLightAccum{};

    Shader *shSurface{}, *shVertexOnly{}, *shGUI{}, *shPostProcess{}, *shCombinationPass{};
    Shader *shDeferredPointLights{};
    Shader *shVolumetricLight;
    Shader *shSimpleGUI{};

    // Local
    Scene *mScene{};

    // Transformation
    TransformComponent *IdentityTransf;
    ConstantBuffer *cbTransform;

    // Geometry Passes
    void Shadows();             // Done
    void GBuffer();             // Done
    void OIT();                 // Done; Optimize sorting; Fix MSAA

    // Occlusion tests
    void CoverageBuffer();

    // Screen-Space Passes
    void Deferred();
    void DeferredLights();

    void SSAO();
    void SSLR();
    void SSLF();
    void FSSSSS();
    void Combine();
    void HDR();
    void VolumetricLight();

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

    //inline Texture* GetTexture(uint32_t index) const override { return s_material.texture_list[index]; };
    inline Texture* GetTexture(TextureList index) const { return s_material.ti.texture_list[(uint32_t)index]; };
};
