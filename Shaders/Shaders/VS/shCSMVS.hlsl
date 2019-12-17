cbuffer MeshBuffer : register(b0) {
    #include "../../../DirectX11 Engine 2019/Engine/Model/Components/Transform.h"
}

struct VS {
    float3 Position : POSITION0;
};

struct GS {
    float4 Position : SV_Position0;
};

GS main(VS In) {
    GS Out;
        Out.Position = mul(mWorld, float4(In.Position, 1.f));
    return Out;
}
