#include "../Common/OITCommon.h"

uint PackColor(float4 color) {
	return (uint(color.r * 255.f) << 24) | 
           (uint(color.g * 255.f) << 16) | 
           (uint(color.b * 255.f) <<  8) | 
            uint(color.a * 255.f);
}

struct PS {
    float4 Position : SV_Position;
    float3 Normal   : NORMAL0;
    float2 Texcoord : TEXCOORD0;
    float4 WorldPos : TEXCOORD1;
};

[earlydepthstencil]
float4 main(PS In, uint coverage : SV_COVERAGE, bool front : SV_IsFrontFace) : SV_TARGET0 {
	uint head = sbLinkedLists.IncrementCounter();
	if( head == 0xFFFFFFFF ) return 0.f;
    
    float4 color = ComputeTransparentColor(input, frontFace);
    
	uint oldHeadBuffVal;
	InterlockedExchange(rwListHead[uint2(In.Position.xy)], head, oldHeadBuffVal);
	
    float Depth;
    
    // Store
	ListItem node;
	    node.uColor    = PackColor(color.rgba);
	    node.uDepth    = asuint(Depth);
	    node.uNext     = oldHeadBuffVal;
        node.uCoverage = coverage;
	sbLinkedLists[head] = node;
    
    return 0.f;
}
