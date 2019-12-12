#include "PrimitiveVS.hlsli"

struct PS {
    float4 Position : SV_Position;
};

PS main(uint index : SV_VertexID) {
    float4 p = float4(0.f, 0.f, 1.f, 1.f);

    // 0   1
    // 
    // 3   2

    // Vertices: 0, 1, 3
    p.xy = _PositionStart;

    [flatten] if( index == 1 ) { p.x = _PositionEnd.x; } // Vertices: 1
    [flatten] if( index == 3 ) { p.y = _PositionEnd.y; } // Vertices: 3
    [flatten] if( index == 2 ) { p.xy = _PositionEnd; }  // Vertices: 2

    PS Out;
        Out.Position = mul(mProj, mul(mView, mul(mWorld, p)));
    return Out;
}
