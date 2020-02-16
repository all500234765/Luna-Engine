cbuffer cBuffer : register(b0) {
    float _LumScale;
    uint _UseAvg;
    float _MaxLum;
    float1 dummy;
};

Texture2D _Texture               : register(t0);
Texture2D _LUT                   : register(t1);
StructuredBuffer<float4> _AvgLum : register(t2);

SamplerState _Sampler1 : register(s0); // Texture
SamplerState _Sampler2 : register(s1); // LUT

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
};

static const float3 LumScale = float3(.299f, .587f, .114f);

float4 main(PS In): SV_Target0 {
    float lum = dot(_Texture.Sample(_Sampler1, In.Texcoord).rgb, LumScale);
    
    /*[branch] if( _UseAvg ) {
        lum *= _LumScale / _AvgLum[0].r;
    } else {
        lum /= _MaxLum;
    }*/
    if( In.Texcoord.y >= .95f ) lum = In.Texcoord.x;
    return _LUT.Sample(_Sampler2, float2(saturate(lum), 0.f));
}
