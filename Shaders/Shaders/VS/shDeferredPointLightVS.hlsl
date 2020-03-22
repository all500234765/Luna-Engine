cbuffer MatrixBuffer : register(b0) {
    #include "Camera.h"
};

struct PointLight {
    #include "PointLight.h"
};

StructuredBuffer<PointLight> sbPointLights : register(t0);

struct VS {
    float3 Position : POSITION0;
    float2 Texcoord : TEXCOORD0;
    float3 Normal   : NORMAL0;
};

struct PS {
    float4 Position  : SV_Position0;
    float4 LightPos  : TEXCOORD0;
	float4 Color     : TEXCOORD1;
    uint InstanceID  : TEXCOORD2;
    float3 ClipSpace : TEXCOORD3;
};

PS main(VS In, uint Instance : SV_InstanceID) {
    PointLight light = sbPointLights[Instance];
    
    float4 P = float4(In.Position * light._LightRadius + light._LightPosition, 1.f);
    
    PS Out;
        Out.Position   = mul(mProj0, mul(mView0, P));
        Out.InstanceID = Instance;
        Out.Color      = float4(light._LightColor, light._LightPower);
        Out.LightPos   = float4(light._LightPosition, light._LightRadius);
        Out.ClipSpace = Out.Position.xyw;
    return Out;
}
