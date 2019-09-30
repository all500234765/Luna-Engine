static const float2 arrPos[6] = {
    // Tri 1
    float2(+1., +1.), // Top Right
    float2(+1., -1.), // Bottom Right
    float2(-1., +1.), // Top Left
    
    // Tri 2
    float2(-1., +1.), // Top Left
    float2(+1., -1.), // Bottom Right
    float2(-1., -1.)  // Bottom Left
};

static const float2 arrUV[6] = {
    // Tri 1
    float2(1., 0.), // Top Right
    float2(1., 1.), // Bottom Right
    float2(0., 0.), // Top Left

    // Tri 2
    float2(0., 0.), // Top Left
    float2(1., 1.), // Bottom Right
    float2(0., 1.)  // Bottom Left
};

struct GS {
    float4 Position : SV_Position;
    float1 Radius   : TEXCOORD0;
    float4 Color    : TEXCOORD1;
};

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
    float4 Color    : TEXCOORD1;
};

cbuffer _Geometry : register(b0) {
    float1 _AspectRatio; // w / h
    float3 _Padding;
}

[maxvertexcount(6)]
void main(point GS In[1], inout TriangleStream<PS> Stream) {
    PS Out;

    [unroll] for( int i = 0, idx = 0; i < 2; i++ ) {
        [unroll] for( int j = 0; j < 3; j++, idx++ ) {
            float2 p = In[0].Position.xy + arrPos[idx] * In[0].Radius * float2(1.f, _AspectRatio);

            Out.Position = float4(p, 1.f, 1.f);
            Out.Texcoord = arrUV[idx];
            Out.Color    = In[0].Color;

            Stream.Append(Out);
        }

        Stream.RestartStrip();
    }
}
