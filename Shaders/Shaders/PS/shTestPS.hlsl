Texture2D _DiffuseTexture    : register(t0);
SamplerState _DiffuseSampler : register(s0);

Texture2D _DepthTexture    : register(t1);
SamplerState _DepthSampler : register(s1);

Texture2D _NoiseTexture    : register(t2);
SamplerState _NoiseSampler : register(s2);

struct PS {
    float4 Position : SV_Position;
    float3 Normal   : NORMAL0;
    float2 Texcoord : TEXCOORD0;
    float4 WorldPos : TEXCOORD1;
    float4 LightPos : TEXCOORD2;
};

float SampleShadow(float4 lpos) {
    float3 projCoords = float3((+1. + lpos.x / lpos.z) * .5, 
                               (-3. + lpos.y / lpos.z) * .5, 
                               lpos.z);

    float sDepth = _DepthTexture.Sample(_DepthSampler, projCoords.xy + _NoiseTexture.Sample(_NoiseSampler, projCoords.xy).rg).r;

    const float bias = .005f;
    return (sDepth - bias) > projCoords.z ? 1. : 0.;
}

float4 main(PS In) : SV_Target0 {
    float3 N = normalize(In.Normal);
    float S = SampleShadow(In.LightPos);

    float L = clamp(dot(normalize(float3(10., 10., 0.)), N), .3f, 1.f) * S;
    return float4((1. + In.LightPos.x / In.LightPos.z) * .5, 
                  (3. - In.LightPos.y / In.LightPos.z) * .5, 0., 1.); // _DiffuseTexture.Sample(_DiffuseSampler, In.Texcoord) /*float4(.7, .9, 0., 1.)*/ * float4(L.xxx, 1.);
}
