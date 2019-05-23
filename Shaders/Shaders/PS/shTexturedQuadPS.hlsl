Texture2D _Texture    : register(t0);
SamplerState _Sampler : register(s0) {
    BorderColor = 0.;
};

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
};

float4 main(PS In) : SV_Target0 {
    float4 Diff = _Texture.Sample(_Sampler, In.Texcoord);

    return Diff;
}
