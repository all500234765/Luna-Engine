#include "DownscaleConstantBuffer.hlsli"

cbuffer _FinalPass : register(b1) {
    // Eye adaptation / HDR
    float1 _MiddleGrey;
    float1 _LumWhiteSqr;

    // Bloom
    float1 _BloomScale;
    
    float1 _Alignment2;

    // DoF
    // _ProjValues.x = ;
    float2 _ProjValues; // _ProjValues.y / _ProjValues.x
    float2 _DoFFarValues;

    // Bokeh
    float1 _ColorScale;
    float1 _RadiusScale;
    float1 _BokehThreshold;

    float1 _Alignment3;
};

RWTexture2D<float4> _Texture    : register(u0);
StructuredBuffer<float> _AvgLum : register(t0);

float3 EyeAdaptationNtoneMapping(float3 HDR) {
    float3 LScale = dot(HDR, _LumFactor.rgb);

    LScale *= _MiddleGrey / _AvgLum[0];
    LScale = (LScale + LScale * LScale / _LumWhiteSqr) / (1.f + LScale);

    // Apply lum scale
    return HDR * LScale;
}

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
    uint2 CurPixel = uint2(dispatchThreadID.x % _Res.x, dispatchThreadID.x / _Res.x);

    if( CurPixel.y < _Res.y ) {
        float4 Color = _Texture[CurPixel];

        // Tonemapping
        Color.rgb = EyeAdaptationNtoneMapping(Color.rgb);

        // Store
        _Texture[CurPixel] = Color*4; // Debug
    }
}
