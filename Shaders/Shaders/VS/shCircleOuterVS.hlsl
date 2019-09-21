#include "PrimitiveVS.hlsli"

struct PS {
    float4 Position : SV_Position;
};

#define PI 3.1415f
#define Tau (2.f * PI)

PS main(uint index : SV_VertexID) {
    float4 p = float4(0.f, 0.f, 1.f, 1.f);

    float fAngle = Tau * (float)index / (float)_Vertices;
    p.xy = _PositionStart + _Radius * float2(cos(fAngle), sin(fAngle));

    PS Out;
        Out.Position = mul(mProj, mul(mView, mul(mWorld, p)));
    return Out;
}
