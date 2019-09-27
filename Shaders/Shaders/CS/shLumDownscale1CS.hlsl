#include "DownscaleConstantBuffer.hlsli"

Texture2D _HDR : register(t0);

RWStructuredBuffer<float> _AvgLum : register(u0); // Intermidiate buffer
RWTexture2D<float4>       _HDRDS  : register(u1);

// For storing intermidiate values
groupshared float _SharedPositions[1024];

float DownScale4x4(uint2 CurPixel, uint groupThreadID) {
    float avgLum = 0.f;

    // Skip out of bounds
    if( CurPixel.y < _Res.y ) {
        // Sum group of 4x4 pixels
        int3 nFullResPos = int3(CurPixel * 4, 0);
        float4 DownScaled = 0.f;

        [unroll] for( int i = 0; i < 4; i++ ) {
            [unroll] for( int j = 0; j < 4; j++ ) {
                DownScaled += _HDR.Load(nFullResPos, int2(j, i));
            }
        }

        DownScaled *= 1.f / 16.f;

        // Downscale HDR
        _HDRDS[CurPixel] = DownScaled;

        // Calc luminance
        avgLum = dot(DownScaled, _LumFactor);

        // Store value
        _SharedPositions[groupThreadID] = avgLum;
    }
    
    // Sync
    GroupMemoryBarrierWithGroupSync();

    return avgLum;
}

float DownScale1024to4(uint dispatchThreadID, uint groupThreadID, float avgLum) {
    [unroll] for( uint groupSize = 4, step1 = 1, step2 = 2, step3 = 3; groupSize < 1024; 
                       groupSize *= 4, step1 *= 4, step2 *= 4, step3 *= 4 ) {
        // Skip out of bounds
        if( groupThreadID % groupSize == 0 ) {
            // Calc lum sum
            float stepAvgLum = avgLum;
            stepAvgLum += (dispatchThreadID + step1) < _Domain ? _SharedPositions[groupThreadID + step1] : avgLum;
            stepAvgLum += (dispatchThreadID + step2) < _Domain ? _SharedPositions[groupThreadID + step2] : avgLum;
            stepAvgLum += (dispatchThreadID + step3) < _Domain ? _SharedPositions[groupThreadID + step3] : avgLum;

            // 
            avgLum = stepAvgLum;
            _SharedPositions[groupThreadID] = stepAvgLum;
        }

        // Sync
        GroupMemoryBarrierWithGroupSync();
    }

    return avgLum;
}

void DownScale4to1(uint dispatchThreadID, uint groupThreadID, uint groupID, float avgLum) {
    if( groupThreadID == 0 ) {
        // Calc avg lum
        float FinalAvgLum = avgLum;
        FinalAvgLum += (dispatchThreadID + 256) < _Domain ? _SharedPositions[groupThreadID + 256] : avgLum;
        FinalAvgLum += (dispatchThreadID + 512) < _Domain ? _SharedPositions[groupThreadID + 512] : avgLum;
        FinalAvgLum += (dispatchThreadID + 768) < _Domain ? _SharedPositions[groupThreadID + 768] : avgLum;

        FinalAvgLum *= 1.f / 1024.f;

        // Write final to UAV for next step
        _AvgLum[groupID] = FinalAvgLum;
    }
}

// Downscale 1st pass
[numthreads(1024, 1, 1)]
void main(uint3 groupID          : SV_GroupID, 
          uint3 dispatchThreadID : SV_DispatchThreadID, 
          uint3 groupThreadID    : SV_GroupThreadID) {
    uint2 CurPixel = uint2(dispatchThreadID.x % _Res.x, dispatchThreadID.x / _Res.x);

    // Reduce group of 16 pixels into a single pixel and store in shared memory
    float avgLum = DownScale4x4(CurPixel, groupThreadID.x);

    // Downscale from 1024 to 4
    avgLum = DownScale1024to4(dispatchThreadID.x, groupThreadID.x, avgLum);

    // Downscale from 4 to 1
    DownScale4to1(dispatchThreadID.x, groupThreadID.x, groupID.x, avgLum);
}
