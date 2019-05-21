Texture2D _DiffuseTexture    : register(t0);
SamplerState _DiffuseSampler : register(s0);

Texture2D<float1> _DepthTexture : register(t1);
SamplerState _DepthSampler      : register(s1);

Texture2D<float2> _NoiseTexture : register(t2);
SamplerState _NoiseSampler      : register(s2);

TextureCube _CubemapTexture  : register(t3);
SamplerState _CubemapSampler : register(s3);

struct PS {
    float4 Position : SV_Position;
    float3 Normal   : NORMAL0;
    float2 Texcoord : TEXCOORD0;
    float4 WorldPos : TEXCOORD1;
    float4 LightPos : TEXCOORD2;
    float3 InputPos : TEXCOORD3;
};

float SampleShadow(float4 lpos) {
    const float bias = 1.f / 2048.f;

    float3 projCoords = float3((1. + lpos.x / lpos.z) * .5,
                               (3. - lpos.y / lpos.z) * .5,
                                    (lpos.z / lpos.w) - bias);

    if( saturate(projCoords.x) != projCoords.x || saturate(projCoords.y) != projCoords.y ) return 0.;

    float sDepth = _DepthTexture.Sample(_DepthSampler, projCoords.xy + _NoiseTexture.Sample(_NoiseSampler, projCoords.xy)*0.);
    
    return (projCoords.z < sDepth) ? 1. : 0.;
}

float4 main(PS In) : SV_Target0 {
    /*return float4((1. + In.LightPos.x / In.LightPos.z) * .5, 
                  (3. - In.LightPos.y / In.LightPos.z) * .5, 0., 1.);*/
    
    float3 N = normalize(In.Normal);
    float S = 1.; // SampleShadow(In.LightPos) * .2 + .8;
    float4 Diff = _DiffuseTexture.Sample(_DiffuseSampler, In.Texcoord);
    //Diff = pow(_CubemapTexture.Sample(_CubemapSampler, normalize(In.InputPos)), 1. / 2.2);

    float L = clamp(dot(normalize(float3(10., 10., 0.)), N), .3f, 1.f);
    return lerp(float4(.5, .6, .8, 1.), Diff, S) * float4(L.xxx, 1.);
}
