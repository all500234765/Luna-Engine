#include "PrimitiveVS.hlsli"

struct PS {
    float4 Position : SV_Position;
};

PS main(uint index : SV_VertexID) {
    float4 p = float4(0.f, 0.f, 1.f, 1.f);

    [branch] if( index == 0 || index == 3 ) { p.xy = _PositionStart; }
    [branch] if( index == 1 ) { p.xy = _PositionEnd  ; }
    [branch] if( index == 2 ) { p.xy = _Alignment    ; }

    PS Out;
        Out.Position = mul(mProj, mul(mView, mul(mWorld, p)));
    return Out;
}
