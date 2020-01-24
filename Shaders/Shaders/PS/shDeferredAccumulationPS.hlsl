#include "../Common/Deferred/Light.hlsli"

Texture2D _DeferredLightTexture    : register(t5);
SamplerState _DeferredLightSampler : register(s5);

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
};

half4 main(PS In) : SV_Target0 {
    float3 Light = _DeferredLightTexture.Sample(_DeferredLightSampler, In.Texcoord);
    Surface surf = GetSurface(In.Texcoord, float2(In.Texcoord.x, 1.f - In.Texcoord.y) * 2.f - 1.f);
    
    return float4(PBRAccumullation(surf, Light), 1.f);
}

// TODO: Move to compute shader

