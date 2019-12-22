#include "RenderTarget.h"

// MSAA Resolve
Shader*         RenderTargetMSAA::g_shMSAADepthResolve = 0;
ConstantBuffer* RenderTargetMSAA::g_MSAAConstantBuffer = 0;
//Texture* RenderTargetMSAA::g_MSAATextureUAV = 0;
