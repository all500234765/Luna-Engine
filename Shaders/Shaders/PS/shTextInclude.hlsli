cbuffer EffectBuffer : register(b0) {
    // Glow


    // Outline


    // Color


    // Etc...


};

Texture2D _FontAtlas      : register(t0);
SamplerState _FontSampler : register(s0);

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
};
