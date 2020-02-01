cbuffer cBuffer : register(b0) {
    #include "../Common/UIInclude.h"    
};

struct VS {
    float3 Position : POSITION;
    float4 Color    : COLOR0;
};

struct PS {
    float4 Position : SV_Position;
    float4 Color    : COLOR0;
};

PS main(VS In) {
    PS Out;
        Out.Position = mul(mProj, float4(In.Position, 1.f));
        Out.Color    = In.Color;
    return Out;
}
