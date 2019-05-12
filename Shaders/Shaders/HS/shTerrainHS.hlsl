//cbuffer TessellationBuffer {
    static const float tessellationAmount = 12.f;
    //float3 padding;
//};

struct HS {
    float3 Position : POSITION0;
    float3 Normal   : NORMAL0;
    float2 Texcoord : TEXCOORD0;
};

struct DS {
    float3 Position : POSITION0;
    float3 Normal   : NORMAL0;
    float2 Texcoord : TEXCOORD0;
};

struct TS {
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

TS Path(InputPatch<HS, 3> inputPatch, uint patchId : SV_PrimitiveID) {
    TS Out;
        // Set the tessellation factors for the three edges of the triangle.
        Out.edges[0] = tessellationAmount;
        Out.edges[1] = tessellationAmount;
        Out.edges[2] = tessellationAmount;
        // Set the tessellation factor for tessallating inside the triangle.
        Out.inside = tessellationAmount;
    return Out;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("Path")]
DS main(InputPatch<HS, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID) {
    DS Out;
        Out.Position = patch[pointId].Position;
        Out.Normal   = patch[pointId].Normal;
        Out.Texcoord = patch[pointId].Texcoord;
    return Out;
}
