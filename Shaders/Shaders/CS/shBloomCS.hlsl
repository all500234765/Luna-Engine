#include "DownscaleConstantBuffer.hlsli"

Texture2D<float4>       _HDRDS  : register(t0);
StructuredBuffer<float> _AvgLum : register(t1);

RWTexture2D<float4> _Bloom : register(u0);

[numthreads(1024, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
    uint2 CurPixel = uint2(dispatchThreadID.x % _Res.x, dispatchThreadID.x / _Res.x);

    if( CurPixel.y < _Res.y ) {
        float4 Color = _HDRDS.Load(int3(CurPixel, 0));
        float Lum    = dot(Color, _LumFactor);
        float avgLum = _AvgLum[0];

        // Find color scale
        float colorScale = saturate(Lum - avgLum * _BloomThreshold);

        // Store
        _Bloom[CurPixel] = Color * colorScale;
    }
}
