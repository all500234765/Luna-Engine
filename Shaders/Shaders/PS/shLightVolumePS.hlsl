cbuffer Light : register(b0) {
    
    float3 _EyePosition;
    float3 _LightDir;
    float3 _LightIntensity;
};

struct PS {
    float4 Position : SV_Position;
    float4 WorldPos : TEXCOORD0;
};

float4 main(PS In, bool Front : SV_FrontFace) : SV_Target {
    float Sign      = mad(Front, 2.f, -1.f);
    float4 WorldPos = In.WorldPos;
    float3 V        = (WorldPos.xyz - _EyePosition);
    float EyeDist   = length(V);
    
    // Normalize
    V *= rcp(EyeDist);
    
    float3 L = _LightDir;
    float3 integral = Integrate_SDirectional(EyeDist, V, L);

    return float4(Sign * integral * _LightIntensity, 0.f);
}