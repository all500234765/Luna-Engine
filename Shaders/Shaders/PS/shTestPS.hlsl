cbuffer cbBoolTextures : register(b0) {
    bool bDiffuse;
    bool bNormals;
    bool bOpacity;
    bool bSpecular;
    bool bCubemap;
    bool PADDING[11];
};

Texture2D _DiffuseTexture    : register(t0);
SamplerState _DiffuseSampler : register(s0);

Texture2D _NormalTexture    : register(t1);
SamplerState _NormalSampler : register(s1);

Texture2D _OpacityTexture    : register(t2);
SamplerState _OpacitySampler : register(s2) {
    BorderColor = float4(1.f, 1.f, 1.f, 1.f);
};

Texture2D _SpecularTexture    : register(t3);
SamplerState _SpecularSampler : register(s3) {
    BorderColor = float4(0.f, 0.f, 0.f, 1.f);
};

Texture2D<float1> _DepthTexture       : register(t4);
SamplerComparisonState  _DepthSampler : register(s4) {
    Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    ComparisonFunc = LESS;
};

Texture2D<float2> _NoiseTexture : register(t5);
SamplerState _NoiseSampler      : register(s5);

TextureCube _CubemapTexture  : register(t6);
SamplerState _CubemapSampler : register(s6);

struct PS {
    float4   Position : SV_Position;
    float3x3 WorldTBN : TEXCOORD0;
    float2   Texcoord : TEXCOORD3;
    float3   WorldPos : TEXCOORD4;
    float4   LightPos : TEXCOORD5;
    float3   InputPos : TEXCOORD6;
    float3   ViewDir  : TEXCOORD7;
};

float SampleShadow(float4 lpos) {
    const float _ShadowMapTexel = 1.f / 2048.f;
    const float bias = .001*0;// +_ShadowMapTexel;
    const float _Far = 10000.f;

    float3 projCoords = float3(.5 + (lpos.x / lpos.w) * .5,
                               .5 - (lpos.y / lpos.w) * .5,
                                    (lpos.z / lpos.w));

    // Apply bias
    projCoords.z -= bias;

    // If out of bounds - no shadows should be applied
    if( saturate(projCoords.x) != projCoords.x || saturate(projCoords.y) != projCoords.y
     || projCoords.z > 1. ) return 0.;

    // Calculate dx and dy based on Noise
    float2 fNoise = _NoiseTexture.Sample(_NoiseSampler, projCoords.xy /* 20.f*/) * 2. - 1.; // * .00625;
    float2 dx = float2(fNoise.x * _ShadowMapTexel, 0.);
    float2 dy = float2(0., fNoise.y * _ShadowMapTexel);
    float2 p_dxdy = (dx + dy);
    float2 n_dxdy = (dx - dy);

    // 
    float2 samples[8] = { dx, -dx, dy, -dy, p_dxdy, -p_dxdy, n_dxdy, -n_dxdy };

    // 
    float sDepth = 0.;

    [unroll(8)]
    for( int i = 0; i < 8; i++ )
        sDepth += _DepthTexture.SampleCmpLevelZero(_DepthSampler, projCoords.xy + samples[i], projCoords.z);

    return sDepth * .125;
}

struct GBuffer {
    half4 Diffuse  : SV_Target0;
    half4 Normal   : SV_Target1;
    half4 Specular : SV_Target2;
};

// https://aras-p.info/texts/CompactNormalStorage.html#method03spherical
// Spherical Coordinates
#define kPI 3.1415926536f
half2 EncodeNormal(half3 n) {
    return half2(atan2(n.y, n.x) / kPI, n.z) * .5f + .5f;
}

GBuffer main(PS In) {
    [branch] if( _OpacityTexture.Sample(_OpacitySampler, In.Texcoord).r < .1 ) { discard; }

    // Calculate normal
    half3 N;
    //[branch] if( bNormals ) 
    {
        half3 NormalTex = _NormalTexture.Sample(_NormalSampler, In.Texcoord).rgb;

        N = normalize(mul(In.WorldTBN, NormalTex * 2. - 1.));
    } /*else {
        N = normalize(In.WorldTBN._m20_m21_m22);
    } //*/

    // Shadows
    const float s = .4;
    half S = 1.; // SampleShadow(In.LightPos) * s + (1. - s);

    // Diffuse texture
    half4 Diff = _DiffuseTexture.Sample(_DiffuseSampler, In.Texcoord);

    //Diff = pow(_CubemapTexture.Sample(_CubemapSampler, normalize(In.InputPos)), 1. / 2.2);

    const half3 _LightPos = half3(100.f, 10.f, 0.f);
    const half3 _LightColor = half3(.7f, .9f, .8f);

    // Phong
    //half3 lDir = normalize(_LightPos - In.WorldPos.xyz);
    //half3 vDir = normalize(In.ViewDir);
    //half3 hDir = normalize(lDir + vDir);

    //half L = clamp(dot(N, lDir), .3f, 1.f);           // Light intensity
    //half spec = pow(max(dot(N, hDir), 0.f), 32.f); // Light specular lightning
    
    //half3 SpecularColor = spec * _LightColor;

    GBuffer Out;
        Out.Normal   = half4(EncodeNormal(N), 0., 1.);
        Out.Diffuse  = half4(lerp(.21 * half3(.5, .6, .8), Diff.rgb, S), Diff.a);// * half4(L.xxx, 1.) + half4(SpecularColor, 0.);
        Out.Specular = _SpecularTexture.Sample(_SpecularSampler, In.Texcoord);
    return Out;
}
