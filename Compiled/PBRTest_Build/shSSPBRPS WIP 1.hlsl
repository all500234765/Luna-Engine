// Settings
cbuffer SSPBRSettingsBuffer : register(b0) {
    float1 _Metalness;
    float3 _LightPos;
    float1 _Rougness;
    float1 _Albedo;
    float2 _Texel;
    float3 _Repeats;
    float1 _Exposure;
    float3 _LightDiffuse;
    float1 _Dummy2;
};

// Textures
Texture2D<float1> _Heightmap : register(t0);
SamplerState _HeightmapSampl : register(s0);

TextureCube<float3> _Cubemap : register(t1);
SamplerState _CubemapSampler : register(s1);

#define PI 3.14159265f
#define InvPI (1.f / PI)

struct PS {
    float4 Position  : SV_Position;
    float2 Texcoord  : TEXCOORD0;
    float4 CameraPos : TEXCOORD1;
};

// Convert Heightmap to Normal map
float3 Height2Normal(float2 uv) {
    float dx0 = _Heightmap.Sample(_HeightmapSampl, uv + float2(_Texel.x, 0.f));
    float dy0 = _Heightmap.Sample(_HeightmapSampl, uv + float2(0.f, _Texel.y));

    float dx1 = _Heightmap.Sample(_HeightmapSampl, uv - float2(_Texel.x, 0.f));
    float dy1 = _Heightmap.Sample(_HeightmapSampl, uv - float2(0.f, _Texel.y));

    float dzdx = (dx0 - dx1) / (2.f * _Texel.x);
    float dzdy = (dy0 - dy1) / (2.f * _Texel.y);

    return normalize(float3(-dzdx, -dzdy, 1.f));
}

// Old
float OrenNayar(float3 lDir, float3 vDir, float3 Normal, float NdotL, float3 NdotV) {
    float LdotV = dot(lDir, vDir);

    // 
    float s = LdotV - NdotL * NdotV;
    float t = lerp(1.f, max(NdotL, NdotV), step(0.f, s));

    float s2 = _Rougness * _Rougness;
    float1 A = 1.f + s2 * (_Albedo / (s2 + .13f) + .5f / (s2 + .33f));
    float1 B = .45f * s2 / (s2 + .09f);

    // 
    return _Albedo * max(.1, NdotL) * (A + B * s / t) * InvPI;
}

float BlinnPhong(float3 lDir, float3 vDir, float3 Normal) {
    // 
    float3 H = normalize(lDir + vDir);

    // Distance
    float dist = length(lDir);
    dist *= dist;

    // 
    float NdotH = dot(Normal, H);

    // Calculate factors
    float P = 32.f * _Metalness;
    float T = pow(saturate(NdotH), P);

    // 
    return T * P / dist;
}

// IBL / PBR
float3 FresnelShlick(float cTheta, float3 F0) {
    return mad(max((float3) (1.f - _Rougness), F0) -F0, pow(1.f - cTheta, 5.f), F0);
}

float DistrGGX(float3 N, float3 H) {
    float r4 = pow(_Rougness * _Rougness, 2.f);
    float NdotH = saturate(dot(N, H));
    float NdotH2 = pow(NdotH, 2.f);
	
    float d = PI * pow(mad(NdotH2, (r4 - 1.f), 1.f), 2.f);
	
    return r4 / d;
}

float GSchlickGGX(float NdotV) {
    float r = _Rougness + 1.f;
    float k = pow(r, 2.f) * .125f;

    float d = mad(NdotV, 1.f - k, k);
	return NdotV / d;
}

float GSmith(float3 N, float3 V, float3 L) {
    float NdotV = saturate(dot(N, V));
    float NdotL = saturate(dot(N, L));
	
    float ggx1 = GSchlickGGX(NdotL);
    float ggx2 = GSchlickGGX(NdotV);

    return ggx1 * ggx2;
}

float4 main(PS In): SV_Target0 {
    // Modified UVs
    float2 uv = In.Texcoord * _Repeats.xy;

    // Height and Normal at current UV
    float1 HeightOrig = _Heightmap.Sample(_HeightmapSampl, uv);
    float1 Height = mad(HeightOrig, 2.f, - 1.f);
    float3 Normal = Height2Normal(uv);
	
	// Position
    float3 P = float3(uv, Height);

    // L2UV ray direction
    float3 RayDir = _Repeats * (_LightPos - float3(In.Texcoord, Height));

    // Ray marcher settings
    const float Steps = 128.f;
    const float Step = 1.f / Steps;

    // A bit of optimization
    Height += _LightPos.z;

    // Ray march shadow
    float S = 1.f;
    [unroll(int(Steps))] for( float t = Step; t <= 1.f; t += Step ) {
        float1 z = mad(_Heightmap.Sample(_HeightmapSampl, uv + RayDir.xy * t), 2.f, - 1.f);

        S = min(S, Height - z);
    }

    // Normalize ray direction
    RayDir = normalize(RayDir);

    // ViewDir and ViewVector
    const float3 ViewDir = float3(0.f, 0.f, 1.f);
    float3 V = normalize(ViewDir - P);

	// Dot Products
    float1 NdotV  = dot(Normal, ViewDir);
    float1 NdotV2 = dot(Normal, V);
    float1 NdotL  = dot(Normal, RayDir);
	
    float3 H = normalize(RayDir + V);

	// PBR
    float3 F0 = lerp(.04f, _Albedo, _Metalness);

    //float Dist = dot(P - _LightPos, P - _LightPos);
    //float invD = 1.f / Dist;

    float Dist = length(P - _LightPos);
    float invD = 1.f / (Dist * Dist); // TODO: Use inverse sqrt (rsqrt)
	
    float3 Radiance = _LightDiffuse * invD;
	
	// Cook-Torrance BRDF
    float  NDF = DistrGGX(Normal, H);
	float  G   = GSmith(Normal, V, RayDir);
    float3 F   = FresnelShlick(saturate(dot(H, V)), F0);

    float3 KS = F;
    float3 KD = (1.f - KS) * (1.f - _Metalness); // TODO: MAD

    float3 N = NDF * G * F;
    float1 D = mad(4.f * saturate(NdotV2), saturate(NdotL), .001f); // .001f: Prevent division by zero
    float3 Spec = N / D;

	// Light
    float3 Light = (KD * _Albedo * InvPI + Spec) * Radiance * saturate(NdotL);

    // IBL
    KS = FresnelShlick(saturate(NdotV2), F0);
    KD = (1.f - KS) * (1.f - _Metalness);
    
    float3 Irradiance = _Cubemap.Sample(_CubemapSampler, Normal);
    float3 Diffuse    = Irradiance * _Albedo;
    float3 Ambient    = KD / Diffuse;

	// TODO: MAD
    //float3 Amb = .04f * _Albedo;
    float3 Result = Ambient + Light;

    // Env. mapping
    //float3 R = reflect(normalize(P - ViewDir), Normal);
    //float3 EnvMapping = _Cubemap.Sample(_CubemapSampler, R) * _Exposure;

	// IBL
	//
    //float3 FS    = FresnelShlick(max(NdotV2, 0.f), F0);
    //float3 irrad = _Cubemap.SampleLevel(_CubemapSampler, RayDir, 2);
	//float3 diff  = irrad * _Albedo;
	//float3 ambi  = (1.f - FS) * diff;

    // Final steps
    //float3 Diff = Result/*diff/*max(OrenNayar(RayDir, ViewDir, Normal, NdotL, NdotV)
    //                 + BlinnPhong(RayDir, ViewDir, Normal)
    //                   , .1f) * _LightDiffuse + EnvMapping*/;
    
	// Reinhard Tonemapping w/ gamma correction
    Result = Result / (1.f + Result);

    // Apply shadow
    S *= max(.1f, NdotL);
    S = max(S, .1f);

    // Output
    return float4(Result * S, 1.f);
}
