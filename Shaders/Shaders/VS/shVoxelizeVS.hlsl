cbuffer MatrixBuffer : register(b0) {
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
    float4   vPosition;
};

cbuffer Voxels : register(b1) {
    float4x4 mVoxel;
}

struct VS {
    float3 Position  : POSITION0;
};

struct PS {
    float4 Position : SV_Position;
    float4 GridPos  : TEXCOORD0;
};

PS main(VS In) {
    float4 WorldPos = mul(mWorld, float4(In.Position , 1.f));
    
    PS Out;
        Out.Position = mul(mProj, mul(mView, WorldPos));
        Out.GridPos  = mul(mVoxel, float4(In.Position, 1.f));
    return Out;
}
