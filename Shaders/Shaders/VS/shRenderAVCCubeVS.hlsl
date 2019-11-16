struct VS {
    float3 Position : POSITION0;
    float1 Padding  : TEXCOORD0;
};

struct GS {
    float4 Position : SV_Position0;
};

GS main(VS In) {
    GS Out;
        Out.Position = float4(In.Position, 1.f);
    return Out;
}
