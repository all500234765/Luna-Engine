cbuffer MatrixBuffer : register(b0) {
    #include "Camera.h"
};

cbuffer GlobalBuffer : register(b1) {
    #include "Deferred/Global.h"
};

struct PointLight {
    #include "PointLight.h"
};

StructuredBuffer<PointLight> sbPointLights : register(t0);

struct ConstantOut {
    float Edges[4]  : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
};

struct PS {
    float4 Position : SV_Position0;
    float2 Texcoord : TEXCOORD0;
    float4 LightPos : TEXCOORD1;
	float4 Color    : TEXCOORD2;
    uint InstanceID : TEXCOORD3;
    float3 Normal   : NORMAL0;
};

struct HS {
    float3 Position : POSITION0;
    uint Instance   : SV_InstanceID;
};

[domain("quad")]
PS main(ConstantOut In, float2 uv : SV_DomainLocation, const OutputPatch<HS, 4> quad) {
    // Light data
    PointLight light = sbPointLights[quad[0].Instance];
    float4 P = float4(light._LightPosition, light._LightRadius);
    
    // Transform uv to clip-space
    float2 clip = uv * 2. - 1.;
    
    // Find absolute max distance from center
    float2 absDist = abs(clip);
    float len = max(absDist.x, absDist.y);

    // Generate final pos in clip space
    float3 nDir = normalize(float3(clip, len - 1.) * quad[0].Position);
    float4 pos = float4(P.xyz + nDir * P.w * quad[0].Instance, 1.);

    // Transform to projected space and generate UVs
    PS Out;
        Out.Position = mul(mProj0, mul(mView0, pos));
        Out.LightPos = P;
		Out.Color    = float4(light._LightColor, light._LightPower);

        // Normalize to get texcoords
        Out.Texcoord = Out.Position.xy / Out.Position.w * .5;
        Out.Texcoord.x += .5;
        Out.Texcoord.y = .5 - Out.Texcoord.y;
        
        Out.InstanceID = quad[0].Instance;
        Out.Normal = nDir;
    return Out;
}
