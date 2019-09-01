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

float3 Height2Normal(float2 uv) {
    float dx0 = _Heightmap.Sample(_HeightmapSampl, uv + float2(_Texel.x, 0.f));
    float dy0 = _Heightmap.Sample(_HeightmapSampl, uv + float2(0.f, _Texel.y));

    float dx1 = _Heightmap.Sample(_HeightmapSampl, uv - float2(_Texel.x, 0.f));
    float dy1 = _Heightmap.Sample(_HeightmapSampl, uv - float2(0.f, _Texel.y));

    float dzdx = (dx0 - dx1) / (2.f * _Texel.x);
    float dzdy = (dy0 - dy1) / (2.f * _Texel.y);

    return normalize(float3(-dzdx, -dzdy, 1.f));
}

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

float3 FresnelShlick(float cTheta, float3 F0) {
	// TODO: Add MAD here
    return F0 + (max((float3) (1.f - _Rougness), F0) -F0) * pow(1.f - cTheta, 5.f);
}

float4 main(PS In): SV_Target0 {
    // Modified UVs
    float2 uv = In.Texcoord * _Repeats.xy;

    // Height and Normal at current UV
    float1 HeightOrig = _Heightmap.Sample(_HeightmapSampl, uv);
    float1 Height = HeightOrig * 2.f - 1.f;
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
        float1 z = _Heightmap.Sample(_HeightmapSampl, uv + RayDir.xy * t) * 2.f - 1.f;

        S = min(S, Height - z);
    }

    // Normalize ray direction
    RayDir = normalize(RayDir);

    // View dir and View Vector
    const float3 ViewDir = float3(0.f, 0.f, 1.f);
    float3 V = normalize(ViewDir - P);

	// Dot Products
    float1 NdotV  = dot(Normal, ViewDir);
    float1 NdotV2 = dot(Normal, V);
    float1 NdotL  = dot(Normal, RayDir);

    // Env. mapping
    float3 R = reflect(normalize(P - ViewDir), Normal);
    float3 EnvMapping = _Cubemap.Sample(_CubemapSampler, R) * _Exposure;

	// IBL
	float3 F0    = lerp(.04f, _Albedo, _Metalness);
    float3 FS    = FresnelShlick(max(NdotV2, 0.f), F0);
    float3 irrad = _Cubemap.SampleLevel(_CubemapSampler, RayDir, 2);
	float3 diff  = irrad * _Albedo;
	float3 ambi  = (1.f - FS) * diff;

    // Final steps
    float3 Diff = diff/*max(OrenNayar(RayDir, ViewDir, Normal, NdotL, NdotV)
                     + BlinnPhong(RayDir, ViewDir, Normal)
                       , .1f) * _LightDiffuse + EnvMapping*/;
    
    // Apply shadow
    S *= max(.1f, NdotL);
    S = max(S, .05f);

    // Output
    return float4(Diff * S, 1.f);
}
