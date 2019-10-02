Texture2D<float1> _BokehTex : register(t0);
SamplerState _LinearSampler : register(s0);

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
    float4 Color    : TEXCOORD1;
};

float4 main(PS In) : SV_Target0 {
    return float4(0.f, 0.f, 0.f, 1.f); //In.Color * _BokehTex.Sample(_LinearSampler, In.Texcoord);
}
