cbuffer SpotLight : register(b0) {
    #include "SpotLight.h"
};

cbuffer Camera : register(b1) {
    #include "Camera.h"
};

#define CylinderPortion .2f
#define ExpandAmount (1.f + CylinderPortion)

struct HSConstant {
    float Edges[4]  : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
};

struct HS {
    float3 Position : POSITION0;
};

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
};

[domain("quad")]
PS main(HSConstant In, float2 uv : SV_DomainLocation, const OutputPatch<HS, 4> quad) {
    // Transform UVs to clip space
    float2 clip = uv * float2(2.f, -2.f) + float2(-1.f, 1.f);
    
    // Find vertex offsets, based on UVs
    float2 offset = abs(clip);
    float max_len = max(offset.x, offset.y);
    
    // Force cone vertices to the mesh edge
    float2 unorm_clip = saturate(clip * ExpandAmount);
    float max_len_unc = max(unorm_clip.x, unorm_clip.y);
    float2 sign_clip  = sign(clip) * unorm_clip;
    
    // Convert positions to half sphere
    // With cone vertices on the edge
    float3 half_sphere = normalize(float3(unorm_clip, 1.f - max_len_unc));
    
    // Scale the sphere to the size of the cone's rounded base
    half_sphere = normalize(float3(half_sphere.xy * _LightSinA, _LightCosA));
    
    // Find offsets of the cone vertices
    float cyll_zoff = saturate((max_len * ExpandAmount - 1.f) / CylinderPortion);
    
    // Offset the cone vertices to their final position
    float4 P = float4(half_sphere.xy * (1.f - cyll_zoff), half_sphere.z - cyll_zoff * _LightCosA, 1.f);
    
    // Transform to View Space
    PS Out;
        Out.Position = mul(mProj0, mul(mView0, P));
        Out.Texcoord = Out.Position.xy / Out.Position.w * float2(.5f, -.5f) + .5f;
    return Out;
}
