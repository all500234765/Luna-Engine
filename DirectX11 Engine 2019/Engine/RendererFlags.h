#pragma once

enum RendererFlags {
    None         = 0, 
    DepthPass    = 1, 
    OpacityPass  = 2, 
    OpaquePass   = 4, 
    RenderSkybox = 8, 

};
