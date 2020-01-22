struct ConstantOut {
    float Edges[4]  : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
};

ConstantOut HemisphereConst() {
    const float fTessFactor = 9.f; // 18.f

    ConstantOut Out;
        Out.Edges[0] = Out.Edges[1] = Out.Edges[2] = Out.Edges[3] = fTessFactor;
        Out.Inside[0] = Out.Inside[1] = fTessFactor;
    return Out;
}

static const float3 HemiDir[2] = {
    float3(1., 1., 1.), float3(-1., 1., -1.)
};

struct HS {
    float3 Position : POSITION0;
    uint Instance   : SV_InstanceID;
};

struct VS {
    uint InstanceID : TEXCOORD0;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HemisphereConst")]
HS main(in InputPatch<VS, 1> patch, in uint PatchID : SV_PrimitiveID) {
    HS Out;
        Out.Position = HemiDir[PatchID];
        Out.Instance = patch[0].InstanceID;
    return Out;
}
