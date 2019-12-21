#pragma once

#include "RendererBase.h"
#include "Engine Includes/MainInclude.h"

#include "Effects/HDRPostProcess.h"
#include "Effects/SSAOPostProcess.h"
#include "Effects/SSLRPostProcess.h"
#include "Effects/SSLFPostProcess.h"
#include "Effects/CascadeShadowMapping.h"
#include "Effects/CoverageBuffer.h"
#include "Effects/OrderIndendentTransparency.h"

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
            Texture *checkboard;
            Texture *bluenoise_rg_512;
            Texture *checkboard;
        } tex;

        struct {
            SamplerState point;
            SamplerState linear;
            SamplerState point_comp;
            SamplerState linear_comp;
        } sampl;

    } s_material;

    // TODO: Do clustered rendering
    CubemapTexture *mCubemap;


    // Effects
    HDRPostProcess                 *gHDRPostProcess;
    SSAOPostProcess                *gSSAOPostProcess;
    SSLRPostProcess                *gSSLRPostProcess;
    SSLFPostProcess                *gSSLFPostProcess;
    CascadeShadowMapping<3, false> *gCascadeShadowMapping;
    CoverageBuffer                 *gCoverageBuffer;
    OrderIndendentTransparency     *gOrderIndendentTransparency;

    SSLRArgs    gSSLRArgs;
    SSAOArgs    gSSAOArgs;
    CSMArgs     gCSMArgs;
    CBuffArgs   gCBuffArgs;
    OITSettings gOITSettings;

    // Resources
    RenderTarget2DColor1DepthMSAA *rtDepth;
    RenderTarget2DColor4DepthMSAA *rtGBuffer;
    RenderTarget2DColor2DepthMSAA *rtTransparency;

    Shader *shSurface, *shVertexOnly;

    // Local
    Scene *mScene;

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
    void DOF();
    void HDR();

    // Final Passes
    void Final();

public:
    void Init() override;
    void Render() override;
    void Release() override;
    void ImGui() override;
};
