cbuffer _xMatrices : register(b0) {
    float4x4 _mWorldViewProj;
    float4x4 _mWorldView;
};

struct PS {
    float4 Position : SV_Position0;
    float4 ViewPos  : TEXCOORD0;
    float3 ClipSPos : TEXCOORD1; // Clip space position
    float3 ViewNorm : NORMAL0;
};

struct VS {
    float3 Position : POSITION0;
    float3 Normal   : NORMAL0;
};

PS main(VS In) {
    float4 p = float4(In.Position, 1.f);

    PS Out;
        Out.Position = mul(_mWorldViewProj, p);
        Out.ViewPos  = mul(_mWorldView, p);
        Out.ViewNorm = mul((float3x3)_mWorldView, In.Normal);
        Out.ClipSPos = Out.Position.xyz / Out.Position.w;
    return Out;
}
