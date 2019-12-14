#include "../Common/OITCommon.h"

struct PS {
    float4 Position : SV_Position;
    float3 Normal   : NORMAL0;
    float2 Texcoord : TEXCOORD0;
    float4 WorldPos : TEXCOORD1;
};

float4 ComputeTransparentColor(PS In, bool front) {
    //abs(In.Position.x - In.Position.y - front * 2.f).xxx * 10.f
    return front ? float4(.7f, .9f, 0.f, .25f) : float4(0.f, 0.f, 1.f, .25f);
}

[earlydepthstencil]
float4 main(PS In, uint coverage : SV_COVERAGE, bool front : SV_IsFrontFace) : SV_TARGET0 {
	uint head = sbLinkedLists.IncrementCounter();
	if( head == 0xFFFFFFFF ) return 0.f;
    
    float4 color = ComputeTransparentColor(In, front);
    
	uint oldHeadBuffVal;
	InterlockedExchange(rwListHead[uint2(In.Position.xy)], head, oldHeadBuffVal);
	
    float Depth = In.WorldPos.w;
    
    // Store
	ListItem node;
	    node.uColor    = PackColor(color.rgba);
	    node.uDepth    = asuint(Depth);
	    node.uNext     = oldHeadBuffVal;
        node.uCoverage = coverage;
	sbLinkedLists[head] = node;
    
    return 0.f;
}
