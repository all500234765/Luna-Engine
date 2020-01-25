cbuffer bGlobal : register(b0) {
    #include "Deferred/Global.h"
}

Texture2D<float> _DepthTexture : register(t0);
SamplerState _DepthSampler     : register(s0);

Texture2D<half2> _NormalTexture : register(t1);
SamplerState _NormalSampler     : register(s1);

Texture2D<half4> _ShadingTexture : register(t2);
SamplerState _ShadingSampler     : register(s2);

Texture2D<half3> _IndirectTexture : register(t3);
SamplerState _IndirectSampler     : register(s3);

Texture2D<half4> _AlbedoTexture : register(t4);
SamplerState _AlbedoSampler     : register(s4);

// https://aras-p.info/texts/CompactNormalStorage.html#method03spherical
// Spherical Coordinates
#define kPI 3.1415926536f
half3 NormalDecode(half2 enc) {
    half2 scth, ang = enc * 2.f - 1.f;
    sincos(ang.x * kPI, scth.x, scth.y);

    half2 scphi = half2(sqrt(1.f - ang.y * ang.y), ang.y);
    return half3(scth.y * scphi.x, scth.x * scphi.x, scphi.y);
}

float Depth2Linear(float z) {
    return _ProjValues.x / (z + _ProjValues.y);
}

float3 GetWorldPos(float2 ClipSpace, float z) {
    return mul(_mInvView, float4(ClipSpace * _ProjValues.zw * z, z, 1.)).xyz;
}

struct Surface {
    //                  | Red         | Green        | Blue        | Alpha        | Format   
    //                  +-------------+--------------+-------------+--------------+----------
    float4 Albedo;   // | Base color in linear space               | Shadow       | RGBA16F
    //                  +------------------------------------------+--------------+----------
    float3 Indirect; // | Indirect light                           | Unused       | RGBA16F
    //                  +------------------------------------------+--------------+----------
    float3 Normal;   // | Normal                                   | Unused       | RGBA16F
    //                  +------------------------------------------+--------------+----------
    float3 WorldPos; // | World position                           | Unused       | Internal
    //                  +-------------+--------------+-------------+--------------+----------
    float4 Shading;  // | Metallic    | Roughness    | AO          | Unused       | RGBA16F
    //                  +-------------+--------------+-------------+--------------+----------
    
    // TODO: Format
    float3 ViewPos;
    float3 ViewDir;
    float2 UV;
};

Surface GetSurface(float2 uv, float2 ClipSpace) {
    Surface surf;
        surf.Albedo   = _AlbedoTexture.Sample(_AlbedoSampler, uv);
        surf.Indirect = _IndirectTexture.Sample(_IndirectSampler, uv);
        surf.Shading  = _ShadingTexture.Sample(_ShadingSampler, uv);
        surf.Normal   = NormalDecode(_NormalTexture.Sample(_NormalSampler, uv));
        surf.WorldPos = GetWorldPos(ClipSpace, Depth2Linear(1.f - _DepthTexture.Sample(_DepthSampler, uv)));
        
        surf.ViewPos  = _mInvView._m03_m13_m23;
        surf.UV       = uv;
        surf.ViewDir  = surf.ViewPos - surf.WorldPos;
    return surf;
}

struct PointLight {
    #include "PointLight.h"
};

#define InvPI (1.f / kPI)

// IBL / PBR
float3 FresnelShlick(float cTheta, float3 F0, float R) {
    return mad(max((float3) (1.f - R), F0) -F0, pow(1.f - cTheta, 5.f), F0);
}

float DistrGGX(float3 N, float3 H, float R) {
    float r4 = pow(R * R, 2.f);
    float NdotH = saturate(dot(N, H));
    float NdotH2 = pow(NdotH, 2.f);
	
    float d = kPI * pow(NdotH2 * (r4 - 1.f) + 1.f, 2.f);
	
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

float4 DeferredPBR(Surface surf, PointLight light) {
    // Vectors
    float3 L = normalize(light._LightPosition - surf.WorldPos);
    float3 V = normalize(surf.ViewDir);
    float3 H = normalize(L + V);
    float3 N = surf.Normal;
    
    // Shading parameters
    float Roughness        = surf.Shading.g;
    float Metallic         = surf.Shading.r;
    float AmbientOcclusion = surf.Shading.b;
    
    // Cosines
    float NdotV = dot(N, V);
    float NdotL = dot(N, L);

    // PBR
    float3 F0 = lerp((.04f).xxx, surf.Albedo.rgb, surf.Shading.r);
    
    float Dist = length(light._LightPosition - surf.WorldPos) / (light._LightRadius);
    float invD = saturate(1.f - Dist * 1.f); //1.f / (Dist * Dist); // TODO: Use inverse sqrt (rsqrt)
    
    float3 Radiance = /*_WorldLightColor */ invD.xxx; // + Dist + 0*Dist / 20.f; //invD;

    // Cook-Torrance BRDF
    float  NDF = DistrGGX(N, H, Roughness);
	float  G   = GSmith(N, V, L, Roughness);
    float3 F   = FresnelShlick(saturate(dot(H, V)), F0, Roughness);

    float3 KS = F;
    float3 KD = (1.f - KS) * (1.f - Metallic); // TODO: MAD

    float3 N_ = NDF * G * F;
    float1 D  = mad(4.f * saturate(NdotV), saturate(NdotL), .001f); // .001f: Prevent division by zero
    float3 Spec = N_ / D;

	// Light
    float3 Light = light._LightColor * (KD * surf.Albedo.rgb * InvPI + Spec) * Radiance * saturate(NdotL);
    
//return float4((1.f - Dist).xxx, 1.f);

    return float4(pow(Light, 1.f / 2.2f), 1.f);
}

float3 PBRAccumullation(Surface surf, float3 Light) {
    // Vectors
    float3 V = normalize(surf.ViewDir);
    float3 N = surf.Normal;
    
    // Cosines
    float NdotV = dot(N, V);
    
    // Shading parameters
    float Roughness        = surf.Shading.g;
    float Metallic         = surf.Shading.r;
    float AmbientOcclusion = surf.Shading.b;

    // IBL
    float3 F0 = lerp((.04f).xxx, surf.Albedo.rgb, surf.Shading.r);
    float3 KS = FresnelShlick(saturate(NdotV), F0, Roughness);
    float3 KD = (1.f - KS) * (1.f - Metallic);
    
    float3 Diffuse    = surf.Indirect * surf.Albedo.rgb;
    float3 AmbientIBL = KD * Diffuse;
    
    float3 Ambient = AmbientIBL * AmbientOcclusion;
    
    // Accumulation                  Shadows
    float3 Acc = (Ambient + Light) * surf.Albedo.w;
    return pow(Acc, 1.f / 2.2f);
}
