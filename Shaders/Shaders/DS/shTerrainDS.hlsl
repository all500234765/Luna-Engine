cbuffer MatrixBuffer : register(b0) {
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

struct TS {
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

struct DS {
    float3 Position : POSITION0;
    float3 Normal   : NORMAL0;
    float2 Texcoord : TEXCOORD0;
};

struct PS {
    float4 Position : SV_Position;
    float3 Normal   : NORMAL0;
    float2 Texcoord : TEXCOORD0;
};

[domain("tri")]
PS main(TS In, float3 uvwCoord : SV_DomainLocation, const OutputPatch<DS, 3> path) {
    PS Out;
    float3 Vertex, Normal;

    Vertex = uvwCoord.x * path[0].Position +
             uvwCoord.y * path[1].Position +
             uvwCoord.z * path[2].Position;

    Normal = uvwCoord.x * path[0].Normal +
             uvwCoord.y * path[1].Normal +
             uvwCoord.z * path[2].Normal;

    Out.Position = mul(mWorld, float4(Vertex, 1.));
    Out.Position = mul(mView, Out.Position);
    Out.Position = mul(mProj, Out.Position);

    Out.Normal = mul(mWorld, float4(Normal, 0.));
    Out.Texcoord = path[0].Texcoord;

    return Out;
}
