cbuffer MatrixBuffer : register(b0) {
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
    float4   vPosition;
};

cbuffer PrimitiveBuffer : register(b1) {
    float2 _PositionStart;
    float2 _PositionEnd;
};

struct PS {
    float4 Position : SV_Position;
};

PS main(uint index : SV_VertexID) {
    float4 p = float4(0.f, 0.f, 1.f, 1.f);

    // Tri 1
    // 0   1
    // 
    //     2

    // Tri 2
    // 3
    // 
    // 5   4

    // Vertices: 0, 1, 3
    [branch] if( index < 4 && index != 2 ) { p.xy = _PositionStart; }

    [branch] if( index == 1 ) { p.x = _PositionEnd.x; } // Vertices: 1
    [branch] if( index == 5 ) { p.y = _PositionEnd.y; } // Vertices: 5

    // Vertices: 2, 4
    [branch] if( index == 2 || index == 4 ) { p.xy = _PositionEnd; }

    PS Out;
        Out.Position = mul(mProj, mul(mWorld, p));
    return Out;
}
