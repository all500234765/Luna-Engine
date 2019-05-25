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
    float3 ViewDir  : TEXCOORD4;
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

struct GBuffer {
    half4 Diffuse  : SV_Target0;
    half2 Normal   : SV_Target1;
    half4 Specular : SV_Target2;
};

// https://aras-p.info/texts/CompactNormalStorage.html#method03spherical
// Spherical Coordinates
#define kPI 3.1415926536f
half2 EncodeNormal(half3 n) {
    return half2(atan2(n.y, n.x) / kPI, n.z) * .5f + .5f;
}

GBuffer main(PS In) {
    /*return float4((1. + In.LightPos.x / In.LightPos.z) * .5, 
                  (3. - In.LightPos.y / In.LightPos.z) * .5, 0., 1.);*/
    
    half3 N = normalize(In.Normal);
    half S = 1.; // SampleShadow(In.LightPos) * .2 + .8;
    half4 Diff = _DiffuseTexture.Sample(_DiffuseSampler, In.Texcoord);

    //Diff = pow(_CubemapTexture.Sample(_CubemapSampler, normalize(In.InputPos)), 1. / 2.2);

    const half3 _LightPos = half3(100.f, 10.f, 0.f);
    const half3 _LightColor = half3(.7f, .9f, .8f);

    // Phong
    half3 lDir = normalize(_LightPos - In.WorldPos.xyz);
    half3 vDir = normalize(In.ViewDir);
    half3 hDir = normalize(lDir + vDir);

    half L = clamp(dot(N, lDir), .3f, 1.f);           // Light intensity
    half spec = pow(max(dot(N, hDir), 0.f), 32.f); // Light specular lightning
    
    half3 SpecularColor = spec * _LightColor;

    GBuffer Out;
        Out.Normal   = EncodeNormal(N);
        Out.Diffuse  = lerp(half4(.5, .6, .8, 1.), Diff, S) * half4(L.xxx, 1.) + half4(SpecularColor, 0.);
        Out.Specular = half4(Diff.rgb * SpecularColor, spec);
    return Out;
}
