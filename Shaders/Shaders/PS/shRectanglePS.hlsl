cbuffer PrimitiveColorBuffer : register(b0) {
    float4 _Color;
};

struct PS {
    float4 Position : SV_Position;
};

float4 main(PS In) : SV_Target0 {
    return _Color;
}
