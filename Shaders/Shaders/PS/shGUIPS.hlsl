Texture2D    _Texture : register(t0);
SamplerState _Sampler : register(s0);

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
};

float4 main(PS In): SV_Target0 {
    return _Texture.Sample(_Sampler, In.Texcoord);
}
