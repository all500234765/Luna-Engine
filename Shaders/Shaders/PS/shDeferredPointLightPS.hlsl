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

struct PS {
    float4 Position  : SV_Position0;
    float2 Texcoord  : TEXCOORD0;
    float4 LightPos  : TEXCOORD1;
	float4 Color     : TEXCOORD2;
    uint InstanceID  : TEXCOORD3;
    float2 ClipSpace : TEXCOORD4;
};

half4 PointLight(float3 p, float3 n, float4 lpos, float4 color) {
    float3 lDist = lpos.xyz               - p;
    float3 eyeD  = _mInvView._m03_m13_m23 - p;
    float1 dist  = length(lDist);
	float3 lDir  = lDist / dist;
	
	// Final color
	float3 final = color.rgb;


    // Lambertian
    final *= saturate(dot(n, lDir));// * _LightDiffuse * _LightData.y;
	

    // Specular
    eyeD = normalize(eyeD);
    float3 H = normalize(eyeD + lDir);
    float NdotH = saturate(dot(H, n));
    final += pow(NdotH, 10.f);

	// Attenuation
    float dist2norm = 1.f - saturate(dist / lpos.w);
    float att = dist2norm * dist2norm;
    final *= att;
	
	return float4(final, att);
}

half4 main(PS In, bool Front : SV_IsFrontFace) : SV_Target0 {
    // Clip if 
    clip(In.LightPos.w < 1.f ? -1.f : 1.f); // Too small
	
    // Unpack GBuffer
    //half4 Diffuse = ;
    float LinDepth = Depth2Linear(1.f - _DepthTexture.Sample(_DepthSampler, In.Texcoord));
    half3 Normal = NormalDecode(_NormalTexture.Sample(_NormalSampler, In.Texcoord));
    
    // Flip normal if we outside
	//Normal *= Front * 2.f - 1.f;
    
    // Reconstruct world position
    float3 WorldPos = GetWorldPos(In.ClipSpace, LinDepth);
	
    // Calculate point light
    return PointLight(WorldPos, Normal, In.LightPos, In.Color);
}
