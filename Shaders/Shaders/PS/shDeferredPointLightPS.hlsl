#include "../Common/Deferred/Light.hlsli"

struct PS {
    float4 Position  : SV_Position0;
    float2 Texcoord  : TEXCOORD0;
    float4 LightPos  : TEXCOORD1;
	float4 Color     : TEXCOORD2;
    uint InstanceID  : TEXCOORD3;
    float2 ClipSpace : TEXCOORD4;
};

half4 main(PS In, bool Front : SV_IsFrontFace) : SV_Target0 {
    // Clip if 
    clip(In.LightPos.w < 1.f ? -1.f : 1.f); // Too small
	
    Surface surf = GetSurface(In.Texcoord, In.ClipSpace);
    PointLight light;
        light._LightPosition = In.LightPos.xyz;
        light._LightRadius   = In.LightPos.w;
        light._LightColor    = In.Color.rgb;
        light._LightPower    = In.Color.w;
    return DeferredPBR(surf, light);
}
