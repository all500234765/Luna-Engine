cbuffer MatrixBuffer : register(b0) {
    #include "Camera.h"
};

struct SpotLight {
    #include "SpotLight.h"
};

StructuredBuffer<SpotLight> sbSpotLight : register(t0);

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
    float3 LightExt0 : TEXCOORD4;
    float3 LightExt1 : TEXCOORD5;
};

PS main(VS In, uint Instance : SV_InstanceID) {
    SpotLight light = sbSpotLight[Instance];
    
    //mRotFromForward(normalize(light._LightDirection), normalize(light._LightUp)), 
    float4 P = float4(mul(light._LightRotMat, In.Position) * light._LightDistance + light._LightPosition, 1.f);
	//P.xyz *= light._LightDistance;
	//P.xyz += light._LightPosition;
    
    //float4 P = mul(mRotFromForward(light._LightDirection), float4(In.Position * light._LightDistance, 1.f));
	//P.xyz += light._LightPosition; // Move light to specific point
    
    PS Out;
        Out.Position   = mul(mProj0, mul(mView0, P));
        Out.InstanceID = Instance;
        Out.Color      = float4(light._LightColor, light._LightPower);
        Out.LightPos   = float4(light._LightPosition, light._LightDistance);
        Out.ClipSpace = Out.Position.xyw;
        Out.LightExt0 = float3(light._LightCutOff, light._LightOutCutOff, 0.f);
        Out.LightExt1 = light._LightDirection;
    return Out;
}
