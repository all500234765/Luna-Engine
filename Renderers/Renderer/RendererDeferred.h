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
    Shader *shSurface;

    // Geometry Passes
    void Shadows();
    void GBuffer();
    void OIT();
    void GBufferMSAAResolve();

    // Screen-Space Passes
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
    void Render(uint32_t CameraIndex) override;
    void Release() override;
    void ImGui() override;
};
