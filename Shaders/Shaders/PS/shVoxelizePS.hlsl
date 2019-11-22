struct PS {
    float4 Position : SV_Position;
    float4 GridPos  : TEXCOORD0;
};

cbuffer _Data : register(b0) {
    float2 _Stride;
    float2 _Empty;
}

RWBuffer<uint> _VoxelBuf : register(u1);

float4 main(PS In) : SV_Target {
    int3 p = In.GridPos.xyz / In.GridPos.w;
    InterlockedOr(_VoxelBuf[dot(p.xy, _Stride) + (p.z >> 5)], 1 << (p.z & 31));
    
    discard;
    return 0.f;
}
