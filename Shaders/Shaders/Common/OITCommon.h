#define MAX_ELEMENTS 8
#define MAX_FRAGMENTS 32

#ifndef __cplusplus
    #ifndef NoRW
        #define CounterStructuredBuffer RWStructuredBuffer
        #define UAV(x) : register(u[x])
        #define RWTexture_(x) RWTexture2D<x> 
        #define _globallycoherent globallycoherent
    #else
        #define CounterStructuredBuffer StructuredBuffer
        #define UAV(x) : register(t[x])
        #define RWTexture_(x) Texture2D<x> 
        #define _globallycoherent 
    #endif
#else
    #define RWTexture_(x) Texture
    #define UAV(x)
    #define _globallycoherent
#endif

struct ListItem {
    uint  uColor;
    float fDepth;
    uint  uNext;
    uint  uCoverage;
};

_globallycoherent CounterStructuredBuffer<ListItem> sbLinkedLists UAV(1);
_globallycoherent RWTexture_(uint) rwListHead UAV(2);

// 
#ifndef __cplusplus
float4 UnpackColor(uint color) {
    return float4(
        saturate(float((color >> 24) & 0xFF) / 0xFF), 
        saturate(float((color >> 16) & 0xFF) / 0xFF), 
        saturate(float((color >>  8) & 0xFF) / 0xFF), 
        saturate(float( color        & 0xFF) / 0xFF)
    );
}

struct NodeItem {
    uint uColor;
    float fDepth;
};

uint PackColor(float4 color) {
	return ((uint(saturate(color.r) * 255.f)) << 24) | 
           ((uint(saturate(color.g) * 255.f)) << 16) | 
           ((uint(saturate(color.b) * 255.f)) <<  8) | 
            (uint(saturate(color.a) * 255.f));
}
    #undef CounterStructuredBuffer
#else
    #undef globallycoherent 
#endif

#undef RWTexture_
#undef UAV
#undef _globallycoherent
