Texture2D _DiffuseTexture    : register(t0);
SamplerState _DiffuseSampler : register(s0);

Texture2D _SpecularTexture    : register(t1);
SamplerState _SpecularSampler : register(s1);

Texture2D _AccTexture    : register(t2);
SamplerState _AccSampler : register(s2);

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
};

half4 main(PS In) : SV_Target0 {
    // Sample diffuse and specular
    half4 Diff = _DiffuseTexture.Sample(_DiffuseSampler, In.Texcoord);
    half4 Spec = _SpecularTexture.Sample(_SpecularSampler, In.Texcoord);

    half4 Acc = _AccTexture.Sample(_AccSampler, In.Texcoord);

    return half4(Diff + Spec) * Acc;
}
