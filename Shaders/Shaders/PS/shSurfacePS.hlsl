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

Texture2D _SpecularTexture    : register(t3); // Specular / Metalness
SamplerState _SpecularSampler : register(s3) {
    BorderColor = float4(0.f, 0.f, 0.f, 1.f);
};

Texture2D<float1> _DepthTexture       : register(t4);
SamplerComparisonState  _DepthSampler : register(s4) {
    Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    ComparisonFunc = GREATER;
};

Texture2D<float2> _NoiseTexture : register(t5);
SamplerState _NoiseSampler      : register(s5);

TextureCube<float3> _CubemapTexture : register(t6);
SamplerState        _CubemapSampler : register(s6);

Texture2D<float> _RougnessTexture : register(t7); // Rougness
SamplerState     _RougnessSampler : register(s7) {
    BorderColor = float4(0.f, 0.f, 0.f, 1.f);
};

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
    const float bias = -.000005; // +_ShadowMapTexel;
    const float _Far = 10000.f;

    float3 projCoords = float3(.5 + (lpos.x / lpos.w) * .5,
                               .5 - (lpos.y / lpos.w) * .5,
                                    (lpos.z / lpos.w));

    // Apply bias
    projCoords.z -= bias;

    // If out of bounds - no shadows should be applied
    [flatten] if( saturate(projCoords.x) != projCoords.x || saturate(projCoords.y) != projCoords.y
    // || 
	//projCoords.z < 0. 
        ) 
        return 0.;

    // Calculate dx and dy based on Noise
    float2 fNoise = _NoiseTexture.Sample(_NoiseSampler, projCoords.xy * 20.f) * 2. - 1.; // * .00625;
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

#define InvPI (1.f / kPI)

// IBL / PBR
float3 FresnelShlick(float cTheta, float3 F0, float _Rougness) {
    return mad(max((float3) (1.f - _Rougness), F0) -F0, pow(1.f - cTheta, 5.f), F0);
}

float DistrGGX(float3 N, float3 H, float _Rougness) {
    float r4 = pow(_Rougness * _Rougness, 2.f);
    float NdotH = saturate(dot(N, H));
    float NdotH2 = pow(NdotH, 2.f);
	
    float d = kPI * pow(mad(NdotH2, (r4 - 1.f), 1.f), 2.f);
	
    return r4 / d;
}

float GSchlickGGX(float NdotV, float _Rougness) {
    float r = _Rougness + 1.f;
    float k = pow(r, 2.f) * .125f;

    float d = mad(NdotV, 1.f - k, k);
	return NdotV / d;
}

float GSmith(float3 N, float3 V, float3 L, float _Rougness) {
    float NdotV = saturate(dot(N, V));
    float NdotL = saturate(dot(N, L));
	
    float ggx1 = GSchlickGGX(NdotL, _Rougness);
    float ggx2 = GSchlickGGX(NdotV, _Rougness);

    return ggx1 * ggx2;
}

GBuffer main(PS In, bool bIsFront : SV_IsFrontFace, uint SampleIndex : SV_SampleIndex) {
    const half3 _LightPos = half3(100.f, 10.f, 0.f);
    const half3 _LightColor = half3(.7f, .9f, .8f);

    // Opacity check
    [flatten] if( _OpacityTexture.Sample(_OpacitySampler, In.Texcoord).a < .1 ) { discard; }

    // Calculate normal
    half3 N;
    //[branch] if( bNormals ) 
    {
        half3 NormalTex = _NormalTexture.Sample(_NormalSampler, In.Texcoord).rgb;
        //[branch] if( NormalTex.r > 0. ) 
            N = normalize(mul(In.WorldTBN, NormalTex * 2.f - 1.f));
        //else 
            N = normalize(In.WorldTBN._m20_m21_m22);

        // Flip normals
        //N *= (1. - bIsFront * 2.);
    } /*else {
        N = normalize(In.WorldTBN._m20_m21_m22);
    } //*/

    // Shadow mapping
    const float s = .5;
    half S = SampleShadow(In.LightPos) * s + (1. - s);

    // Sample diffuse texture
    half4 Diff = _DiffuseTexture.Sample(_DiffuseSampler, In.Texcoord);
    half Metal = _SpecularTexture.Sample(_SpecularSampler, In.Texcoord).r;
    half Rough = _RougnessTexture.Sample(_RougnessSampler, In.Texcoord);

    //Diff.rgb = pow(Diff.rgb, 2.2f);

    // Vectors
    float3 L = normalize(In.LightPos.xyz - In.WorldPos);
    float3 V = normalize(In.ViewDir 	 - In.Position.xyz);
    float3 H = normalize(L + V);

    // 
    float NdotV = dot(N, V);
    float NdotL = dot(N, L);

    // PBR
    float3 F0 = lerp(.04f, Diff.rgb, Metal);
    
    float Dist = length(In.Position - In.LightPos);
    float invD = 1.f / (Dist * Dist); // TODO: Use inverse sqrt (rsqrt)
    
    float3 Radiance = _LightColor * invD;

    // Cook-Torrance BRDF
    float  NDF = DistrGGX(N, H, Rough);
	float  G   = GSmith(N, V, L, Rough);
    float3 F   = FresnelShlick(saturate(dot(H, V)), F0, Rough);

    float3 KS = F;
    float3 KD = (1.f - KS) * (1.f - Metal); // TODO: MAD

    float3 N_ = NDF * G * F;
    float1 D  = mad(4.f * saturate(NdotV), saturate(NdotL), .001f); // .001f: Prevent division by zero
    float3 Spec = N_ / D;

	// Light
    float3 Light = (KD * Diff.rgb * InvPI + Spec) * Radiance * saturate(NdotL);

    // IBL
    KS = FresnelShlick(saturate(NdotV), F0, Rough);
    KD = (1.f - KS) * (1.f - Metal);
    
    float3 Irradiance = _CubemapTexture.SampleLevel(_CubemapSampler, N.xzy, 1).rgb;
    float3 Diffuse    = Irradiance * Diff.rgb;
    float3 Ambient    = KD / Diffuse;

    Diff.rgb = Diffuse;
    //Diff.rgb = N;

    // Shadows
    Diff.rgb *= S;
    
    //Diff.rgb = pow(Diff.rgb, 1.f / 2.2f);

    // 
    GBuffer Out;
        Out.Normal   = half4(EncodeNormal(N), 0., 1.);
        Out.Diffuse  = Diff;
        //Out.Diffuse  = half4(lerp(.21 * half3(.5, .6, .8), Diff.rgb, S), Diff.a);// * half4(L.xxx, 1.) + half4(SpecularColor, 0.);
        Out.Specular = _SpecularTexture.Sample(_SpecularSampler, In.Texcoord);
    return Out;
}


/*
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

Texture2D _SpecularTexture    : register(t3); // Specular / Metalness
SamplerState _SpecularSampler : register(s3) {
    BorderColor = float4(0.f, 0.f, 0.f, 1.f);
};

Texture2D<float1> _DepthTexture       : register(t4);
SamplerComparisonState  _DepthSampler : register(s4) {
    Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    ComparisonFunc = GREATER;
};

Texture2D<float2> _NoiseTexture : register(t5);
SamplerState _NoiseSampler      : register(s5);

TextureCube<float3> _CubemapTexture : register(t6);
SamplerState        _CubemapSampler : register(s6);

Texture2D<float> _RougnessTexture : register(t7); // Rougness
SamplerState     _RougnessSampler : register(s7) {
    BorderColor = float4(0.f, 0.f, 0.f, 1.f);
};

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
    const float bias = .000005; // +_ShadowMapTexel;
    const float _Far = 10000.f;

    float3 projCoords = float3(.5 + (lpos.x / lpos.w) * .5,
                               .5 - (lpos.y / lpos.w) * .5,
                                    (lpos.z / lpos.w));

    // Apply bias
    projCoords.z -= bias;

    // If out of bounds - no shadows should be applied
    if( saturate(projCoords.x) != projCoords.x || saturate(projCoords.y) != projCoords.y
     //|| projCoords.z < 0. 
        ) 
        return 0.;

    // Calculate dx and dy based on Noise
    float2 fNoise = _NoiseTexture.Sample(_NoiseSampler, projCoords.xy * 20.f) * 2. - 1.; // * .00625;
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
        sDepth += _DepthTexture.SampleCmpLevelZero(_DepthSampler, projCoords.xy + samples[i], -projCoords.z);

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

#define InvPI (1.f / kPI)

// IBL / PBR
float3 FresnelShlick(float cTheta, float3 F0, float _Rougness) {
    return mad(max((float3) (1.f - _Rougness), F0) -F0, pow(1.f - cTheta, 5.f), F0);
}

float DistrGGX(float3 N, float3 H, float _Rougness) {
    float r4 = pow(_Rougness * _Rougness, 2.f);
    float NdotH = saturate(dot(N, H));
    float NdotH2 = pow(NdotH, 2.f);
	
    float d = kPI * pow(mad(NdotH2, (r4 - 1.f), 1.f), 2.f);
	
    return r4 / d;
}

float GSchlickGGX(float NdotV, float _Rougness) {
    float r = _Rougness + 1.f;
    float k = pow(r, 2.f) * .125f;

    float d = mad(NdotV, 1.f - k, k);
	return NdotV / d;
}

float GSmith(float3 N, float3 V, float3 L, float _Rougness) {
    float NdotV = saturate(dot(N, V));
    float NdotL = saturate(dot(N, L));
	
    float ggx1 = GSchlickGGX(NdotL, _Rougness);
    float ggx2 = GSchlickGGX(NdotV, _Rougness);

    return ggx1 * ggx2;
}

GBuffer main(PS In, bool bIsFront : SV_IsFrontFace) {
    const half3 _LightPos = half3(100.f, 10.f, 0.f);
    const half3 _LightColor = half3(.7f, .9f, .8f);

    // Opacity check
    [branch] if( _OpacityTexture.Sample(_OpacitySampler, In.Texcoord).r < .1 ) { discard; }

    // Calculate normal
    half3 N;
    //[branch] if( bNormals ) 
    {
        half3 NormalTex = _NormalTexture.Sample(_NormalSampler, In.Texcoord).rgb;
        //[branch] if( NormalTex.r > 0. ) 
            N = normalize(mul(In.WorldTBN, NormalTex * 2.f - 1.f));
        //else 
            N = normalize(In.WorldTBN._m20_m21_m22);

        // Flip normals
        //N *= (1. - bIsFront * 2.);
    } /*else {
        N = normalize(In.WorldTBN._m20_m21_m22);
    } //* /

    // Shadow mapping
    const float s = .5;
    half S = SampleShadow(In.LightPos) * s + (1. - s);

    // Sample diffuse texture
    half4 Diff = _DiffuseTexture.Sample(_DiffuseSampler, In.Texcoord);
    half Metal = _SpecularTexture.Sample(_SpecularSampler, In.Texcoord).r;
    half Rough = _RougnessTexture.Sample(_RougnessSampler, In.Texcoord);

    // Vectors
    float3 L = normalize(In.LightPos - In.WorldPos);
    float3 V = normalize(In.ViewDir - In.Position);
    float3 H = normalize(L + V);

    // 
    float NdotV = dot(N, V);
    float NdotL = dot(N, L);

    // PBR
    float3 F0 = lerp(.04f, Diff, Metal);
    
    float Dist = length(In.Position - In.LightPos);
    float invD = 1.f / (Dist * Dist); // TODO: Use inverse sqrt (rsqrt)
    
    float3 Radiance = _LightColor * invD;

    // Cook-Torrance BRDF
    float  NDF = DistrGGX(N, H, Rough);
	float  G   = GSmith(N, V, L, Rough);
    float3 F   = FresnelShlick(saturate(dot(H, V)), F0, Rough);

    float3 KS = F;
    float3 KD = (1.f - KS) * (1.f - Metal); // TODO: MAD

    float3 N_ = NDF * G * F;
    float1 D  = mad(4.f * saturate(NdotV), saturate(NdotL), .001f); // .001f: Prevent division by zero
    float3 Spec = N_ / D;

	// Light
    float3 Light = (KD * Diff * InvPI + Spec) * Radiance * saturate(NdotL);

    // IBL
    KS = FresnelShlick(saturate(NdotV), F0, Rough);
    KD = (1.f - KS) * (1.f - Metal);
    
    float3 Irradiance = _CubemapTexture.SampleLevel(_CubemapSampler, N.xzy, 1).rgb;
    float3 Diffuse    = Irradiance * Diff;
    float3 Ambient    = KD / Diffuse;

    Diff.rgb = Diffuse;
    //Diff.rgb = N;

    // Shadows
    Diff.rgb *= S;

    // 
    GBuffer Out;
        Out.Normal   = half4(EncodeNormal(N), 0., 1.);
        Out.Diffuse  = Diff;
        //Out.Diffuse  = half4(lerp(.21 * half3(.5, .6, .8), Diff.rgb, S), Diff.a);// * half4(L.xxx, 1.) + half4(SpecularColor, 0.);
        Out.Specular = _SpecularTexture.Sample(_SpecularSampler, In.Texcoord);
    return Out;
}
*/