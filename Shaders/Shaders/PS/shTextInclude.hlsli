cbuffer EffectBuffer : register(b0) {
    // Glow


    // Outline


    // Color


    // Etc...


};

// SDF Settings
static const float _CharWidth = .4f;
static const float _Softening = .1f;

Texture2D _FontAtlas      : register(t0);
SamplerState _FontSampler : register(s0);

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
};

float SDF(float2 uv) {
    float dist = _FontAtlas.Sample(_FontSampler, uv).a;
    float val = smoothstep(_CharWidth, _CharWidth + _Softening, dist);

    return val;
}
