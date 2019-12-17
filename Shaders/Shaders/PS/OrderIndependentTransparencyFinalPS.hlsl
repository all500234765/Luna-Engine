#define NoRW
#include "../Common/OITCommon.h"

cbuffer cbDataBuffer : register(b0) {
    float4x4 _InvViewProj;
    float3 _CameraPos;
    float1 _MinFadeDist2;
    float1 _MaxFadeDist2;
    float3 _Padding0;
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

void InsertionSortMSAA(in uint startIndex, in uint sample, 
                       inout NodeItem sorted[MAX_FRAGMENTS], 
                       out int counter, out float depth) {
	counter = 0;
    depth = sbLinkedLists[startIndex].fDepth;
	uint index = startIndex;
    
	[loop]
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
    
	[loop]
	for( int k = 1; k < MAX_FRAGMENTS; k++ ) {
		int j = k;
		NodeItem t = sorted[k];

		[loop]
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
	clip(index == 0xFFFFFFFF ? -1 : 1);
	
	float3 color = 0.f;
	float alpha = 1.f;
	
    // TODO: Move sorting to the Compute Shader
	NodeItem sorted[MAX_FRAGMENTS];
    
	[unroll(MAX_FRAGMENTS)]
    for( int j = 0; j < MAX_FRAGMENTS; j++ ) { sorted[j] = (NodeItem)0; }
    
    // Sort
	int counter;
    float depth;
	InsertionSortMSAA(index, sample, sorted, counter, depth);

	// Resolve MSAA
	int resolveBuffer[MAX_FRAGMENTS];
	float4 colors[MAX_FRAGMENTS];
	int resolveIndex = -1;
	float prevDepth = -1.f;
    
	[loop]
	for( int i = 0; i < counter; i++ ) {
		//[branch]
        if( sorted[i].fDepth != prevDepth ) {
			resolveIndex = -1;
			resolveBuffer[i] = 1;
			colors[i] = UnpackColor(sorted[i].uColor);
		} else {
			if( resolveIndex < 0 ) resolveIndex = i - 1;

			colors[resolveIndex] += UnpackColor(sorted[i].uColor);
			resolveBuffer[resolveIndex]++;

			resolveBuffer[i] = 0;
		}
        
		prevDepth = sorted[i].fDepth;
	}

	// Gather
	[loop]
	for( int i = 0; i < counter; i++ ) {
		[branch] if( resolveBuffer[i] == 0 ) continue;
        
		float4 c = colors[i] / float(resolveBuffer[i]);
		alpha *= (1.f - c.a);
		color = lerp(color, c.rgb, c.a);
	}
    
    depth = max(depth, 0.f);
    
    // Reconstruct position
    float3 P = ReconstructWorldPos(In.Texcoord, depth);
    
    // Further -> less visible
    alpha *= depth * _MaxFadeDist2;
    
    //xyz, xzy, zxy, zyx, yxz, yzx
    
    // Closer -> less visible
    float3 dwp = P - _CameraPos.xyz;
    float dist = dot(dwp, dwp);
    alpha *= saturate(dist / _MinFadeDist2);
    
    // Screen-door transparency: Discard pixel if below threshold.
    float4x4 ThresholdMatrix = {
        1.0 / 17.0,  9.0 / 17.0,  3.0 / 17.0, 11.0 / 17.0,
        13.0 / 17.0,  5.0 / 17.0, 15.0 / 17.0,  7.0 / 17.0,
        4.0 / 17.0, 12.0 / 17.0,  2.0 / 17.0, 10.0 / 17.0,
        16.0 / 17.0,  8.0 / 17.0, 14.0 / 17.0,  6.0 / 17.0
    };
    
    clip(alpha - ThresholdMatrix[In.Position.x % 4][In.Position.y % 4]);
    
    // Reconstruct normal
    float3 N = normalize(cross(ddx(P.xyz), ddy(P.xyz)));

    OM Out;
        Out.Color  = float4(color, .1f + alpha);
        Out.Normal = float4(EncodeNormal(N.xyz), 0.f, 1.f);
        Out.Depth  = depth;
    return Out;
}
