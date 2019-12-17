cbuffer MeshBuffer : register(b0) {
    #include "../../../DirectX11 Engine 2019/Engine/Model/Components/Transform.h"
}

cbuffer MatrixBuffer : register(b1) {
    #include "../../../DirectX11 Engine 2019/Engine/Model/Components/Camera.h"
};

struct VS {
    float3 Position : POSITION0;
    float2 Texcoord : TEXCOORD0;
    float3 Normal   : NORMAL0;
};

struct PS {
    float4 Position : SV_Position;
};

PS main(VS In) {
    PS Out;
        Out.Position = mul(mProj0, mul(mView0, mul(mWorld, float4(In.Position, 1.))));
    return Out;
}
