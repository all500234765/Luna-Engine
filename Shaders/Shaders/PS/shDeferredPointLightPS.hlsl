#include "../Common/Deferred/Light.hlsli"

struct PS {
    float4 Position  : SV_Position0;
    float4 LightPos  : TEXCOORD0;
	float4 Color     : TEXCOORD1;
    uint InstanceID  : TEXCOORD2;
    float3 ClipSpace : TEXCOORD3;
};

half4 main(PS In, bool Front : SV_IsFrontFace) : SV_Target0 {
    // Clip if 
    clip(In.LightPos.w < 1.f ? -1.f : 1.f); // Too small
	
    float4 ClipSpaceUV = ClipSpaceUVs(In.ClipSpace);
    Surface surf = GetSurface(ClipSpaceUV.xy, ClipSpaceUV.zw);
    PointLight light;
        light._LightPosition = In.LightPos.xyz;
        light._LightRadius   = In.LightPos.w;
        light._LightColor    = In.Color.rgb;
        light._LightPower    = In.Color.w;
    return DeferredPBR(surf, light);
}
