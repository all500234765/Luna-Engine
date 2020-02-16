cbuffer EffectBuffer : register(b0) {
    // Glow


    // Outline


    // Color
    float4 _Color;

    // Etc...


};

// SDF Settings
cbuffer SDFSettings : register(b1) {
    float _CharWidth;
    float _Softening;
    float _BorderWidth;
    float _BorderSoft;
}

Texture2D _FontAtlas      : register(t0);
SamplerState _FontSampler : register(s0);

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
#ifdef _Color_Text_
    float3 Color    : COLOR0;
#endif
};

float SDF(float2 uv) {
    // Inside
    float dist = _FontAtlas.Sample(_FontSampler, uv).a;
    float val = smoothstep(_CharWidth, _CharWidth + _Softening, dist);

    // Border
    //float BorderDist = _FontAtlas.Sample(_FontSampler, uv).a;
    //float BorderVal  = smoothstep(_BorderWidth, _BorderWidth + _BorderSoft, BorderDist);
    //
    //// Overall
    //float total = lerp(val, 1., BorderVal);

    // lerp(_OutlineColor, _Color, val / total);
    return val /*/ total*/;
}

float4 Effects(float4 sampled) {
    if( _Color.a < .2 ) { discard; }
    return _Color * sampled;
}
