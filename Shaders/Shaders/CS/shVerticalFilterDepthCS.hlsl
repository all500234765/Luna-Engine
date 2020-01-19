#include "DownscaleConstantBuffer.hlsli"

Texture2D<float> _Input : register(t0);
RWTexture2D<float> _Out : register(u0);

static const float SampleWeights[13] = {
    .002216f, .008764f, .026995f, .064759f, .120985f, .176033f, .199471f,
    .176033f, .120985f, .064759f, .026995f, .008764f, .002216f
};

#define KernelHalf 6
#define GroupThreads 128

groupshared float4 _SharedInput[GroupThreads];

[numthreads(GroupThreads, 1, 1)]
void main(uint3 GroupID : SV_GroupID, uint GroupIndex : SV_GroupIndex) {
    int2 coord = int2(GroupID.x, GroupIndex - KernelHalf + (GroupThreads - KernelHalf * 2) * GroupID.y);
    coord = clamp(coord, 0, _Res - 1);

    _SharedInput[GroupIndex] = _Input.Load( int3(coord, 0) );

    // Sync
    GroupMemoryBarrierWithGroupSync();

    // Vertical blur
    if( (GroupIndex >= KernelHalf) && (GroupIndex < (GroupThreads - KernelHalf)) && 
        ((GroupIndex - KernelHalf + (GroupThreads - KernelHalf * 2) * GroupID.y) < _Res.y) ) {
        float Out = 0.f;
        
        [unroll]
        for( int i = -KernelHalf; i <= KernelHalf; ++i ) {
            Out += _SharedInput[GroupIndex + i] * SampleWeights[i + KernelHalf];
		}

		_Out[coord] = Out;
    }

}
