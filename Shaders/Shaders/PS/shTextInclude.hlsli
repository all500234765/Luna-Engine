cbuffer EffectBuffer : register(b0) {
    // Glow


    // Outline


    // Color


    // Etc...


};

// SDF Settings
static const float _CharWidth = .4f;
static const float _Softening = .1f;
static const float _BorderWidth = .5f;
static const float _BorderSoft  = .1f;



Texture2D _FontAtlas      : register(t0);
SamplerState _FontSampler : register(s0);

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
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
