cbuffer cbMaterial : register(b0) {
    #include "Material.h"
};

cbuffer cbAmbientLight : register(b1) {
    #include "AmbientLight.h"
};

cbuffer cbBasicFog : register(b2) {
    #include "BasicFog.h"    
};

cbuffer cbDebugDataGBuffer : register(b3) {
    uint tUseMipMapLUT;
    uint3 align;
};

#include "MaterialTextures.h"

Texture2D<float1> _DepthTexture       : register(t8);
SamplerComparisonState  _DepthSampler : register(s8) {
    Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    ComparisonFunc = GREATER;
};

Texture2D<float2> _NoiseTexture : register(t9);
SamplerState _NoiseSampler      : register(s9);

TextureCube<float3> _CubemapTexture : register(t10);
SamplerState        _CubemapSampler : register(s10);

Texture2D _MipMapDebugTexture : register(t15);
SamplerState _MipMapDebugSamp : register(s15);

struct PS {
    float4   Position : SV_Position;
    float3x3 WorldTBN : TEXCOORD0;
    float2   Texcoord : TEXCOORD3;
    float3   WorldPos : TEXCOORD4;
    float4   LightPos : TEXCOORD5;
    float3   LightPs2 : TEXCOORD6;
    float3   ViewDir  : TEXCOORD7;
};

float SampleShadow(float4 lpos, float NdotL) {
    const float _ShadowMapTexel = 1.f / 2048.f;
    const float bias = clamp(NdotL * .005f * tan(acos(NdotL)), 0.f, .01f); //-.000005; // +_ShadowMapTexel;
    const float _Far = 10000.f;

    float3 projCoords = float3(.5 + (lpos.x / lpos.w) * .5,
                               .5 - (lpos.y / lpos.w) * .5,
                                    (lpos.z / lpos.w));

    // Apply bias
    projCoords.z -= -.000005f;
    //projCoords.z -= bias;

    // If out of bounds - no shadow mapping should be applied
    [flatten] if( saturate(projCoords.x) != projCoords.x || saturate(projCoords.y) != projCoords.y ) return 1.f;

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
    half4 Direct   : SV_Target0;
    half4 Normal   : SV_Target1;
    half4 Shading  : SV_Target2;
    half4 Emission : SV_Target3;
    half4 Indirect : SV_Target4;
};

// https://aras-p.info/texts/CompactNormalStorage.html#method03spherical
// Spherical Coordinates
#define kPI 3.1415926536f
half2 EncodeNormal(half3 n) {
    return half2(atan2(n.y, n.x) / kPI, n.z) * .5f + .5f;
}

#define InvPI (1.f / kPI)

// IBL / PBR
float3 FresnelShlick(float cTheta, float3 F0, float R) {
    return mad(max((float3) (1.f - R), F0) -F0, pow(1.f - cTheta, 5.f), F0);
}

float DistrGGX(float3 N, float3 H, float R) {
    float r4 = pow(R * R, 2.f);
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
    //const half3 _LightPos = half3(100.f, 10.f, 0.f);
    //const half3 _LightColor = half3(.7f, .9f, .8f);
    
    // Get normal
    half3 N = normalize(In.WorldTBN._m20_m21_m22), Bump = half3(0.f, 0.f, 0.f);
    [flatten] if( _Norm ) {
        Bump = mad(_NormalTex.Sample(_NormalSampl, In.Texcoord).rgb, 2.f, -1.f);
        
		Bump.g *= mad(bIsFront, 2.f, -1.f);
        float3x3 TBN = In.WorldTBN;
        TBN._m00_m01_m02 = normalize(TBN._m00_m01_m02);
        TBN._m10_m11_m12 = normalize(TBN._m10_m11_m12);
        N = normalize(lerp(N, mul(transpose(TBN), Bump), _NormalMul));
        
        // Flip normals
        //N *= (bIsFront * 2. - 1);
    }

    // Normal map strengh
    N *= _NormalMul;
    
    // Flip normals
    N *= mad(bIsFront, 2.f, -1.f);
    if( _FlipNormals ) N *= -1.f;
    
    // Sample PBR textures
    float3 Albedo   = pow(_AlbedoTex.Sample(_AlbedoSampl, In.Texcoord).rgb, 2.2f) * _AlbedoMul;
    //float1 AOccl    = _AmbientOcclusionTex.Sample(_AmbientOcclusionSampl, In.Texcoord).r * _AmbientOcclusionMul;
    float3 ORM      = _MetallicTex.Sample(_MetallicSampl, In.Texcoord).rgb;
    
#ifdef _DEBUG
    [flatten] if( tUseMipMapLUT & 0x1 ) {
        float4 a = _MipMapDebugTexture.Sample(_MipMapDebugSamp, In.Texcoord);
        Albedo = lerp(Albedo, a.rgb, a.a);
    }
    
    [flatten] if( tUseMipMapLUT & 0x2 ) {
        float4 a = _MipMapDebugTexture.Sample(_MipMapDebugSamp, In.Texcoord);
        Albedo = a.rgb;
    }
#endif
    
    float1 AOccl = 1.f, Metallic, Rougness;
    [branch] if( _Metal & 4 ) {
        // Combined
        AOccl    = ORM.r;
        Rougness = ORM.g;
        Metallic = ORM.b;
    } else {
        // Simple case
        Metallic = ORM.r;
        Rougness = _RoughnessTex.Sample(_RoughnessSampl, In.Texcoord).r;
    }
    
    Metallic *= _MetallnessMul;
    Rougness *= _RoughnessMul;
    
    // Ambient light
    float3 Ambient = _AmbientLightColor * _AmbientLightStrengh;
    
    // Shadow mapping
    const float s = .5f;
    float S = lerp(1.f - s, 1.f, SampleShadow(In.LightPos, 0.f));
    
    // Final result
    GBuffer Out;
        Out.Direct   = float4(Albedo, S);
        Out.Shading  = float4(Metallic, Rougness, AOccl, 1.f);
        Out.Normal   = float4(EncodeNormal(N), 0., 1.);
        Out.Emission = float4(_EmissionTex.Sample(_EmissionSampl, In.Texcoord).rgb * _EmissionMul, 1.f);
        Out.Indirect = float4(_CubemapTexture.SampleLevel(_CubemapSampler, N.xyz, 1), 1.f);
    return Out;
}
