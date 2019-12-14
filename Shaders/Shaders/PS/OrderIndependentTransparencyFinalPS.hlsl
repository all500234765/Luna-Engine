#define NoRW
#include "../Common/OITCommon.h"

cbuffer cbDataBuffer : register(b0) {
    float4x4 _InvViewProj;
}

// https://aras-p.info/texts/CompactNormalStorage.html#method03spherical
// Spherical Coordinates
#define kPI 3.1415926536f
half2 EncodeNormal(half3 n) {
    return half2(atan2(n.y, n.x) / kPI, n.z) * .5f + .5f;
}

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
};

Texture2D _Background : register(t0);

void InsertionSortMSAA(in uint startIndex, in uint sample, 
                       inout NodeItem sorted[MAX_FRAGMENTS], 
                       out int counter, out float depth) {
	counter = 0;
    depth = sbLinkedLists[startIndex].fDepth;
	uint index = startIndex;
    
	[unroll(MAX_FRAGMENTS)]
    for( int i = 0; i < MAX_FRAGMENTS; i++ ) {
		[flatten] if( index != 0xFFFFFFFF ) {
			[flatten] if( sbLinkedLists[index].uCoverage & (1 << sample) ) {
				sorted[counter].uColor = sbLinkedLists[index].uColor;
				sorted[counter].fDepth = sbLinkedLists[index].fDepth;
                
                depth = max(depth, sorted[counter].fDepth);
				counter++;
			}
            
			index = sbLinkedLists[index].uNext;
		}
	}
    
	[unroll(MAX_FRAGMENTS)]
	for( int k = 1; k < MAX_FRAGMENTS; k++ ) {
		int j = k;
		NodeItem t = sorted[k];

		while( sorted[j - 1].fDepth < t.fDepth ) {
			sorted[j] = sorted[j - 1];
			j--;
            
			[flatten] if( j <= 0 ) break;
		}

		[flatten] if( j != k ) sorted[j] = t;
	}
}

struct OM {
    float4 Color  : SV_Target0;
    float4 Normal : SV_Target1;
    float1 Depth  : SV_Depth;
};

float3 ReconstructWorldPos(float2 uv, float z) {
    float4 p = float4(mad(float2(uv.x, 1.f - uv.y), 2.f, -1.f), z, 1.f);
    float4 v = mul(_InvViewProj, p);
    return v.xyz / v.w;
}

OM main(PS In, uint sample : SV_SampleIndex) {
    uint2 upos = uint2(In.Position.xy);
	uint index = rwListHead[upos];
	
    float4 background = _Background[upos];
	float3 color = background.rgb;
	float alpha = background.a;
	
    // TODO: Move sorting to the Compute Shader
	NodeItem sorted[MAX_FRAGMENTS];
    
	/*[unroll(MAX_FRAGMENTS)]
    for( int j = 0; j < MAX_FRAGMENTS; j++ ) {
		sorted[j] = (NodeItem)0;
	}*/
    
    // Sort
	int counter;
    float depth;
	InsertionSortMSAA(index, sample, sorted, counter, depth);

	// Resolve MSAA
	int resolveBuffer[MAX_FRAGMENTS];
	float4 colors[MAX_FRAGMENTS];
	int resolveIndex = -1;
	float prevDepth = -1.f;
    
	[unroll(MAX_FRAGMENTS)]
	for( int i = 0; i < counter; i++ ) {
		[branch] if( sorted[i].fDepth != prevDepth ) {
			resolveIndex = -1;
			resolveBuffer[i] = 1;
			colors[i] = UnpackColor(sorted[i].uColor);
		} else {
			[flatten] if( resolveIndex < 0 ) resolveIndex = i - 1;

			colors[resolveIndex] += UnpackColor(sorted[i].uColor);
			resolveBuffer[resolveIndex]++;

			resolveBuffer[i] = 0;
		}
        
		prevDepth = sorted[i].fDepth;
	}

	// Gather
	[unroll(MAX_FRAGMENTS)]
	for( int i = 0; i < counter; i++ ) {
		[flatten] if( resolveBuffer[i] == 0 ) continue;
        
		float4 c = colors[i] / float(resolveBuffer[i]);
		alpha *= (1.f - c.a);
		color = lerp(color, c.rgb, c.a);
	}
    
    // Reconstruct position
    float3 P = ReconstructWorldPos(In.Position.xy / float2(1366.f, 768.f), depth);
    //float4 P = float4(mad(In.Position.xy / float2(1366.f, 768.f), 2.f, -1.f), depth, 1.f);
    //P = mul(_InvViewProj, P);
    //P.xyz /= P.w;
    
    // Reconstruct normal
    float3 N = normalize(cross(ddx(P.xyz), ddy(P.xyz)));

    OM Out;
        Out.Color  = float4(color, alpha);
        Out.Depth  = max(depth, 0.f);
        Out.Normal = float4(EncodeNormal(N.xyz), 0.f, 1.f);
    return Out;
}
