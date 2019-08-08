cbuffer MatrixBuffer : register(b0) {
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
    float4   vPosition;
};

struct VS {
    float4 Position : SV_Position0;
    float3 Color    : COLOR0;
};

struct PS {
    float4 Position : SV_Position0;
    float2 Texcoord : TEXCOORD0;
    float3 Color    : COLOR0;
};

PS Project(VS In, float2 off, float2 uv) {
    PS Out;
        // Offset and project
        Out.Position = In.Position;
        Out.Position.xy += off;
        Out.Position = mul(mProj, Out.Position);

        // Set UVs and velocity
        Out.Texcoord = uv;
        Out.Color    = In.Color;

    // 
    return Out;
}

[maxvertexcount(4)]
void main(point VS In[1], inout TriangleStream<PS> Stream) {
    VS Out = In[0];

    const float _Size = 9.99f; // Square's size
    float4 e = float4(float2(-1.f, 1.f) * _Size, 0.f, 1.f);

    // Quad
    Stream.Append(Project(Out, e.xx, e.zz));
    Stream.Append(Project(Out, e.xy, e.zw));
    Stream.Append(Project(Out, e.yx, e.wz));
    Stream.Append(Project(Out, e.yy, e.ww));

    // 
    Stream.RestartStrip();
}
