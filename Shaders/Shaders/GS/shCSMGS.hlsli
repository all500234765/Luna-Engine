#ifndef NumCascades
#define NumCascades 3

#pragma message "[shCSMGS.hlsli]: NumCascades is undefined! Using default: 3 cascades"
#endif

cbuffer cbCSM : register(b0) {
    float4x4 _ViewProj[NumCascades];
};

struct GS {
    float4 Position : SV_Position;
    uint RTIndex    : SV_RenderTargetArrayIndex;
};

[maxvertexcount(3 * NumCascades)]
void main(triangle float4 In[3] : SV_Position, inout TriangleStream<GS> Stream) {
    GS Out;

    for( int f = 0; f < NumCascades; f++ ) {
        Out.RTIndex = f;

        for( int v = 0; v < 3; v++ ) {
            Out.Position = mul(_ViewProj[f], In[v]);
            Stream.Append(Out);
        }

        Stream.RestartStrip();
    }
}
