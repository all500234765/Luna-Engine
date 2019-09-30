Texture2D<float4>  _HDR : register(t0);
Texture2D<float> _Depth : register(t1);
StructuredBuffer<float> _AvgLum : register(t2);

struct TBokeh {
    float2 _Position;
    float1 _Radius;
    float4 _Color;
};

AppendStructuredBuffer<TBokeh> _BokehBuffer : register(u0);

#include "DownscaleConstantBuffer.hlsli"

cbuffer _FinalPass : register(b1) {
    // Eye adaptation / HDR
    float1 _MiddleGrey;
    float1 _LumWhiteSqr;

    // Bloom
    float1 _BloomScale;

    // DoF
    // _ProjValues.x = ;
    float1 _ProjValues; // _ProjValues.y / _ProjValues.x
    float2 _DoFFarValues;

    // Bokeh
    float1 _ColorScale;
    float1 _RadiusScale;
    float1 _BokehThreshold;

    float3 _Alignment2;
};

float Depth2Linear(float z) {
    return _ProjValues * z;
}

float3 CircleOfConfusion(float depth) {
    // Calculate CoC and lerp between colors based on CoC
    return saturate((depth - _DoFFarValues.x) * _DoFFarValues.y);
}

[numthreads(1024, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
    uint2 CurPixel = uint2(dispatchThreadID.x % _Res.x, dispatchThreadID.x / _Res.x);

    if( CurPixel.y < _Res.y ) {
        float depth = _Depth[CurPixel];
        
        // Convert depth to linear space
        depth = Depth2Linear(depth);

        // Calculate circle of consusion
        float CoC = CircleOfConfusion(depth);

        if( CoC > _BokehThreshold ) {
            float4 Color = _HDR.Load(int3(CurPixel, 0));
            float Lum    = dot(Color, _LumFactor);
            float avgLum = _AvgLum[0];
            
            // Find color scale
            float colorScale = saturate(Lum - avgLum * _BloomThreshold);

            if( colorScale > 0.f ) {
                TBokeh Bokeh;
                
                // Tonemap color
                Lum *= _MiddleGrey / avgLum;
                Lum = (Lum + Lum * Lum / _LumWhiteSqr) / (1.f + Lum);

                // 
                Bokeh._Position = 2.f * asfloat(CurPixel) / asfloat(_Res) - 1.f;
                Bokeh._Radius   = CoC * _RadiusScale;
                Bokeh._Color    = Color * Lum * _ColorScale;
                Bokeh._Color.a  = Lum;
                
                _BokehBuffer.Append(Bokeh);
            }
        }
    }
}
