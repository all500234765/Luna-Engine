#define NoRW
#include "../Common/OITCommon.h"

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
};

Texture2D _Background : register(t0);

void InsertionSortMSAA(uint startIndex, uint sample, inout NodeItem sorted[MAX_FRAGMENTS], out int counter) {
	counter = 0;
	uint index = startIndex;
    
	[unroll(MAX_FRAGMENTS)]
    for( int i = 0; i < MAX_FRAGMENTS; i++ ) {
		[flatten] if( index != 0xFFFFFFFF ) {
			[flatten] if( sbLinkedLists[index].uCoverage & (1 << sample) ) {
				sorted[counter].uColor = sbLinkedLists[index].uColor;
				sorted[counter].fDepth = asfloat(sbLinkedLists[index].uDepth);
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

float4 main(PS In, uint sample : SV_SAMPLEINDEX) : SV_TARGET0 {
	uint index = rwListHead[uint2(In.Position.xy)];
	
    float4 background = _Background[uint2(In.Position.xy)];
	float3 color = background.rgb;
	float alpha = background.a;
	
    // TODO: Move sorting to the Compute Shader
	NodeItem sorted[MAX_FRAGMENTS];
    
	[unroll(MAX_FRAGMENTS)]
    for( int j = 0; j < MAX_FRAGMENTS; j++ ) {
		sorted[j] = (NodeItem)0;
	}
    
    // Sort
	int counter;
	InsertionSortMSAA(index, sample, sorted, counter);

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

    return float4(color, alpha);
}
