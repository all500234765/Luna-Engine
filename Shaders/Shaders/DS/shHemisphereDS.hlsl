cbuffer cbMatrixBuffer : register(b0) {
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
    float4   vPosition;
};

struct ConstantOut {
    float Edges[4]  : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
};

struct PS {
    float4 Position : SV_Position0;
    float2 Texcoord : TEXCOORD0;
    float4 LightPos : TEXCOORD1;
};

struct HS {
    float3 Position : POSITION0;
};

[domain("quad")]
PS main(ConstantOut In, float2 uv : SV_DomainLocation, const OutputPatch<HS, 4> quad) {
    // Transform uv to clip-space
    float2 clip = uv * 2. - 1.;

    // Find absolute max distance from center
    float2 absDist = abs(clip);
    float len = max(absDist.x, absDist.y);

    // Generate final pos in clip space
    float3 nDir = normalize(float3(clip, len - 1.) * quad[0].Position);
    float4 pos = float4(nDir * vPosition.w, 1.);

    // Transform to projected space and generate UVs
    PS Out;
        Out.Position = mul(mProj, mul(mView, pos));
        Out.Texcoord = Out.Position.xy / Out.Position.w;
        Out.LightPos = vPosition;
    return Out;
}
