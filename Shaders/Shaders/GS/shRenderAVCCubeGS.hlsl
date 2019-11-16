cbuffer MatrixBuffer : register(b0) {
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
    float4   vPosition; // Light x, y, z, range
};

struct GS {
    float4 Position : SV_Position;
};

struct PS {
    float4 Position : SV_Position;
    float3 Normal   : NORMAL0;
    float3 Texcoord : TEXCOORD0;
};

void Emit(float3 p, float3 n, float3 uvw, float4x4 mat, inout TriangleStream<PS> Out) {
    PS data;
        data.Position = mul(mat, float4(p, 1.f));
        data.Normal   = n;
        data.Texcoord = uvw;
    Out.Append(data);
}

static const float _BS = 32.f;

[maxvertexcount(14)]
void main(point GS In[1], inout TriangleStream<PS> Out) {
    float3 Center = In[0].Position.xyz * _BS;
    float3 llf = Center - _BS / 2.f;
    float3 urb = Center + _BS / 2.f;

    const float3 e = float3(0.f, 1.f, -1.f);

    float4x4 mat  = mul(mProj, mView);
    
    Emit(float3(llf.x, urb.y, llf.z), e.xyx, e.zyz, mat, Out);
    Emit(float3(llf.x, urb.y, urb.z), e.xyx, e.zyy, mat, Out);
    Emit(float3(urb.x, urb.y, llf.z), e.xyx, e.yyz, mat, Out);
    Emit(float3(urb.x, urb.y, urb.z), e.xyx, e.yyy, mat, Out);

    Emit(float3(urb.x, llf.y, urb.z), e.yxx, e.yzy, mat, Out);

    Emit(float3(llf.x, urb.y, urb.z), e.xxy, e.zyy, mat, Out);
    Emit(float3(llf.x, llf.y, urb.z), e.xxy, e.zzy, mat, Out);

    Emit(float3(llf.x, urb.y, llf.z), e.zxx, e.zyz, mat, Out);
    Emit(float3(llf.x, llf.y, llf.z), e.zxx, e.zzz, mat, Out);

    Emit(float3(urb.x, urb.y, llf.z), e.xxz, e.yyz, mat, Out);
    Emit(float3(urb.x, llf.y, llf.z), e.xxz, e.yzz, mat, Out);

    Emit(float3(urb.x, llf.y, urb.z), e.yxx, e.yzy, mat, Out);
    
    Emit(float3(llf.x, llf.y, llf.z), e.xzx, e.zzz, mat, Out);
    Emit(float3(llf.x, llf.y, urb.z), e.xzx, e.zzy, mat, Out);

    Out.RestartStrip();
}
