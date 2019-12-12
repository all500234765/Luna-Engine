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

globallycoherent CounterStructuredBuffer<ListItem> sbLinkedLists UAV(0);
globallycoherent RWTexture_(uint) rwListHead UAV(1);

// 
#ifndef __cplusplus
    #undef CounterStructuredBuffer
#else
    #undef globallycoherent
    #undef RWTexture 
#endif

#undef RWTexture_
#undef UAV
