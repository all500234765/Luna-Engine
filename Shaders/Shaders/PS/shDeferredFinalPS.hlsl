Texture2D _DirectLightTex  : register(t0);
Texture2D _AmbientLightTex : register(t1);
SamplerState _Sampler      : register(s0);

Texture2D _AccTexture    : register(t2);
SamplerState _AccSampler : register(s1);

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
};

half4 main(PS In) : SV_Target0 {
    half4 Direct  = _DirectLightTex.Sample(_Sampler, In.Texcoord);
    half4 Ambient = _AmbientLightTex.Sample(_Sampler, In.Texcoord);

    half4 Acc = _AccTexture.Sample(_AccSampler, In.Texcoord);

    return half4(Direct + Ambient) * Acc;
}
