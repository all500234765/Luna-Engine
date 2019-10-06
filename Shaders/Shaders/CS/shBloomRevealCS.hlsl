Texture2D<float3>  _HDR : register(t0);
Texture2D<float> _Depth : register(t1);
StructuredBuffer<float> _AvgLum : register(t2);

struct TBokeh {
    float4 _Color;
    float2 _Position;
    float1 _Radius;
};

AppendStructuredBuffer<TBokeh> _BokehBuffer : register(u0);

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

float Depth2Linear(float z) {
    return _ProjValues.x / (z + _ProjValues.y);
}

float CircleOfConfusion(float depth) {
    // Calculate CoC
    return saturate((depth - _DoFFarValues.x) * _DoFFarValues.y);
}

[numthreads(1024, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
    uint2 CurPixel = uint2(dispatchThreadID.x % _Res.x, dispatchThreadID.x / _Res.x);

    if( CurPixel.y < _Res.y ) {
        float depth = 1.f - _Depth[CurPixel];
        
        if( depth > 0.f ) {
            // Convert depth to linear space
            depth = Depth2Linear(depth);

            // Calculate circle of consusion
            float CoC = CircleOfConfusion(depth);

            if( CoC > _BokehThreshold ) {
                float3 Color = _HDR.Load(int3(CurPixel, 0));
                float Lum    = dot(Color, _LumFactor.xyz);
                float avgLum = _AvgLum[0];
                
                // Find color scale
                float lumfac = saturate(Lum - avgLum * _BloomThreshold);
                
                if( lumfac > 0.f ) {
                    TBokeh Bokeh;
                    
                    // Tonemap color
                    float LScale = Lum * _MiddleGrey / avgLum;
                    LScale = (LScale + LScale * LScale / _LumWhiteSqr) / (1.f + LScale);

                    // 
                    Bokeh._Position = float2(CurPixel.x, _Res.y - CurPixel.y) / _Res * 2.f - 1.f;
                    Bokeh._Radius   = (CoC - _BokehThreshold) * _RadiusScale;
                    Bokeh._Color    = float4(Color.rgb * LScale * lumfac, _ColorScale);
                    
                    _BokehBuffer.Append(Bokeh);
                }
            }
        }
    }
}
