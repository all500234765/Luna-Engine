#include "DownscaleConstantBuffer.hlsli"

#define MAX_GROUPS 64

// For storing intermidiate results
groupshared float _SharedAvgFinal[MAX_GROUPS];

RWStructuredBuffer<float> _AvgLum : register(u0); // Intermidiate buffer
RWStructuredBuffer<float> _FinLum : register(u1); // Average buffer

StructuredBuffer  <float> _PreLum : register(t0); // Previous luminance

// Downscale 2nd pass
[numthreads(MAX_GROUPS, 1, 1)]
void main(uint3 groupID          : SV_GroupID, 
          uint3 dispatchThreadID : SV_DispatchThreadID, 
          uint3 groupThreadID    : SV_GroupThreadID) {
    // Fill shared memory with 1D values
    float avgLum = 0.f;
    if( dispatchThreadID.x < _GroupSize ) {
        avgLum = _AvgLum[dispatchThreadID.x];
    }

    _SharedAvgFinal[dispatchThreadID.x] = avgLum;
    
    // Sync
    GroupMemoryBarrierWithGroupSync();

    // Downscale 64 to 16
    if( dispatchThreadID.x % 4 == 0 ) {
        // Calc lum sum
        float stepAvgLum = avgLum;
        stepAvgLum += (dispatchThreadID.x + 1) < _GroupSize ? _SharedAvgFinal[dispatchThreadID.x + 1] : avgLum;
        stepAvgLum += (dispatchThreadID.x + 2) < _GroupSize ? _SharedAvgFinal[dispatchThreadID.x + 2] : avgLum;
        stepAvgLum += (dispatchThreadID.x + 3) < _GroupSize ? _SharedAvgFinal[dispatchThreadID.x + 3] : avgLum;

        // Store result
        avgLum = stepAvgLum;
        _SharedAvgFinal[dispatchThreadID.x] = stepAvgLum;
    }

    // Sync
    GroupMemoryBarrierWithGroupSync();
    
    // Downscale 16 to 4
    if( dispatchThreadID.x % 16 == 0 ) {
        // Calc lum sum
        float stepAvgLum = avgLum;
        stepAvgLum += (dispatchThreadID.x +  4) < _GroupSize ? _SharedAvgFinal[dispatchThreadID.x +  4] : avgLum;
        stepAvgLum += (dispatchThreadID.x +  8) < _GroupSize ? _SharedAvgFinal[dispatchThreadID.x +  8] : avgLum;
        stepAvgLum += (dispatchThreadID.x + 12) < _GroupSize ? _SharedAvgFinal[dispatchThreadID.x + 12] : avgLum;

        // Store result
        avgLum = stepAvgLum;
        _SharedAvgFinal[dispatchThreadID.x] = stepAvgLum;
    }

    // Sync
    GroupMemoryBarrierWithGroupSync();
    
    // Downscale 4 to 1
    if( dispatchThreadID.x == 0 ) {
        // Calc lum sum
        float finalAvgLum = avgLum;
        finalAvgLum += (dispatchThreadID.x + 16) < _GroupSize ? _SharedAvgFinal[dispatchThreadID.x + 16] : avgLum;
        finalAvgLum += (dispatchThreadID.x + 32) < _GroupSize ? _SharedAvgFinal[dispatchThreadID.x + 32] : avgLum;
        finalAvgLum += (dispatchThreadID.x + 48) < _GroupSize ? _SharedAvgFinal[dispatchThreadID.x + 48] : avgLum;

        finalAvgLum *= 1.f / 64.f;

        // Store result
        // Calculate final luminance
        float AdaptedLum = lerp(_PreLum[0], finalAvgLum, _Adaptation);
        _FinLum[0] = max(.0001f, AdaptedLum);
    }
}
