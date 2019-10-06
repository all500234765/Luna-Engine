struct TBokeh {
    float4 _Color;
    float2 _Position;
    float1 _Radius;
};

StructuredBuffer<TBokeh> _Bokeh : register(t0);


struct GS {
    float4 Position : SV_Position;
    float1 Radius   : TEXCOORD0;
    float4 Color    : TEXCOORD1;
};

GS main(uint index : SV_VertexID) {
    TBokeh bokeh = _Bokeh[index];

    GS Out;
        Out.Position = float4(bokeh._Position, 0.f, 1.f);
        Out.Radius   = bokeh._Radius;
        Out.Color    = bokeh._Color;
    return Out;
}
