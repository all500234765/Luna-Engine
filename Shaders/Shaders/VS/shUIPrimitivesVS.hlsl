cbuffer cBuffer : register(b0) {
    #include "../Common/UIInclude.h"    
};

struct VS {
    float3 Position : POSITION;
    float2 Texcoord : TEXCOORD0;
    uint   States   : TEXCOORD1;
    uint Padding    : TEXCOORD2;
    float4 Color    : COLOR0;
};

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
    float4 Color    : COLOR0;
    uint   States   : TEXCOORD1;
};

PS main(VS In) {
    PS Out;
        Out.Position = mul(mProj, float4(In.Position, 1.f));
        Out.Color    = In.Color;
        Out.States   = In.States;
        Out.Texcoord = In.Texcoord;
    return Out;
}
