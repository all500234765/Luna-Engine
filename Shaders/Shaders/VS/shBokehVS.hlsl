struct TBokeh {
    float4 _Color;
    float2 _Position;
    float1 _Radius;
};

StructuredBuffer<TBokeh> _Bokeh : register(t0);

cbuffer _FinalPass : register(b0) {
    // Eye adaptation / HDR
    float1 _MiddleGrey;
    float1 _LumWhiteSqr;

    // Bloom
    float1 _BloomScale;

    // DoF
    // _ProjValues.x = ;
    float1 _ProjValues; // _ProjValues.y / _ProjValues.x
    float2 _DoFFarValues;

    // Bokeh
    float1 _ColorScale;     // In this shader we need only this values
    float1 _RadiusScale;    // In this shader we need only this values
    float1 _BokehThreshold;

    float3 _Alignment;
};

struct GS {
    float4 Position : SV_Position;
    float1 Radius   : TEXCOORD0;
    float4 Color    : TEXCOORD1;
};

GS main(uint index : SV_VertexID) {
    TBokeh bokeh = _Bokeh[index];

    GS Out;
        Out.Position = float4(bokeh._Position, 1.f, 1.f);
        Out.Radius   = bokeh._Radius;
        Out.Color    = bokeh._Color;
    return Out;
}
