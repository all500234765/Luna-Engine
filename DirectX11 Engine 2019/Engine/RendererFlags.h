#pragma once

enum RendererFlags {
    None             = 0, 
    DepthPass        = 1, 
    OpacityPass      = 2,  // Deprecated!
    OpaquePass       = 4,  // Deprecated!
    RenderSkybox     = 8,  // Deprecated!
    DontBindShaders  = 16, 
    DontBindTextures = 32, 
    DontBindSamplers = 64,
    ShadowPass       = 128

};
