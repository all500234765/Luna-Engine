cbuffer MatrixBuffer : register(b0) {
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

cbuffer PrimitiveBuffer : register(b1) {
    float2 _PositionStart;
    float2 _PositionEnd;
    float2 _Alignment;
    float1 _Radius;
    uint   _Vertices;
};

