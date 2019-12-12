#define MAX_ELEMENTS 8
#define MAX_FRAGMENTS 32


#ifndef __cplusplus
    #define CounterStructuredBuffer RWStructuredBuffer
    #define UAV(x) : register(u[x])
    #define RWTexture_(x) RWTexture2D<x> 
#else
    #define RWTexture_(x) Texture
    #define UAV(x)
    #define globallycoherent
#endif

struct ListItem {
    uint uColor;
    uint uDepth;
    uint uNext;
    uint uCoverage;
};

globallycoherent CounterStructuredBuffer<ListItem> sbLinkedLists UAV(1);
globallycoherent RWTexture_(uint) rwListHead UAV(2);

// 
#ifndef __cplusplus
float4 UnpackColor(uint color) {
    return float4(
        (color >> 24) & 0xFF, 
        (color >> 16) & 0xFF, 
        (color >>  8) & 0xFF, 
         color        & 0xFF
    );
}

struct NodeItem {
    uint uColor;
    float fDepth;
};

uint PackColor(float4 color) {
	return (uint(color.r * 255.f) << 24) | 
           (uint(color.g * 255.f) << 16) | 
           (uint(color.b * 255.f) <<  8) | 
            uint(color.a * 255.f);
}
    #undef CounterStructuredBuffer
#else
    #undef globallycoherent
    #undef RWTexture 
#endif

#undef RWTexture_
#undef UAV
