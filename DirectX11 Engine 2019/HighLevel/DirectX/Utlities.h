#pragma once

#include "pc.h"

#include "Engine/Extensions/Safe.h"
#include "Engine/Utility/Timer.h"
#include "Engine/Profiler/ScopedRangeProfiler.h"
#include "Engine/DirectX/DirectX.h"
#include "Engine/DirectX/Shader.h"
#include "Engine/DirectX/PolygonLayout.h"
#include "Engine/DirectX/ConstantBuffer.h"
#include "Engine/Scene/Texture.h"
#include "Other/DrawCall.h"

extern _DirectX *gDirectX;

namespace LunaEngine {
    // Discard items(SRV, UAV, CB) from shader slots
#pragma region Compute Shader
    template<UINT dim>
    void CSDiscardUAV() {
        ID3D11UnorderedAccessView *pEmpty[dim] = { nullptr };
        gDirectX->gContext->CSSetUnorderedAccessViews(0, dim, pEmpty, 0);
    }

    template<UINT dim>
    void CSDiscardCB() {
        ID3D11Buffer *pEmpty[dim] = { nullptr };
        gDirectX->gContext->CSSetConstantBuffers(0, dim, pEmpty);
    }

    template<UINT dim>
    void CSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = { nullptr };
        gDirectX->gContext->CSSetShaderResources(0, dim, pEmpty);
    }
#pragma endregion

#pragma region Vertex Shader
    template<UINT dim>
    void VSDiscardCB() {
        ID3D11Buffer *pEmpty[dim] = { nullptr };
        gDirectX->gContext->VSSetConstantBuffers(0, dim, pEmpty);
    }

    template<UINT dim>
    void VSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = { nullptr };
        gDirectX->gContext->VSSetShaderResources(0, dim, pEmpty);
    }
#pragma endregion

#pragma region Pixel Shader
    template<UINT dim>
    void PSDiscardCB() {
        ID3D11Buffer *pEmpty[dim] = { nullptr };
        gDirectX->gContext->PSSetConstantBuffers(0, dim, pEmpty);
    }

    template<UINT dim>
    void PSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = { nullptr };
        gDirectX->gContext->PSSetShaderResources(0, dim, pEmpty);
    }
#pragma endregion

#pragma region Geometry Shader
    template<UINT dim>
    void GSDiscardCB() {
        ID3D11Buffer *pEmpty[dim] = { nullptr };
        gDirectX->gContext->GSSetConstantBuffers(0, dim, pEmpty);
    }

    template<UINT dim>
    void GSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = { nullptr };
        gDirectX->gContext->GSSetShaderResources(0, dim, pEmpty);
    }
#pragma endregion

#pragma region Hull Shader
    template<UINT dim>
    void HSDiscardCB() {
        ID3D11Buffer *pEmpty[dim] = { nullptr };
        gDirectX->gContext->HSSetConstantBuffers(0, dim, pEmpty);
    }

    template<UINT dim>
    void HSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = { nullptr };
        gDirectX->gContext->HSSetShaderResources(0, dim, pEmpty);
    }
#pragma endregion

#pragma region Domain Shader
    template<UINT dim>
    void DSDiscardCB() {
        ID3D11Buffer *pEmpty[dim] = { nullptr };
        gDirectX->gContext->DSSetConstantBuffers(0, dim, pEmpty);
    }

    template<UINT dim>
    void DSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = { nullptr };
        gDirectX->gContext->DSSetShaderResources(0, dim, pEmpty);
    }
#pragma endregion
    
#pragma region
    template<>
    void CSDiscardUAV<1>();

    template<>
    void CSDiscardCB<1>();

    template<>
    void CSDiscardSRV<1>();

    template<>
    void VSDiscardCB<1>();

    template<>
    void VSDiscardSRV<1>();

    template<>
    void PSDiscardCB<1>();

    template<>
    void PSDiscardSRV<1>();

    template<>
    void GSDiscardCB<1>();

    template<>
    void GSDiscardSRV<1>();

    template<>
    void HSDiscardCB<1>();

    template<>
    void HSDiscardSRV<1>();

    template<>
    void DSDiscardCB<1>();

    template<>
    void DSDiscardSRV<1>();
#pragma endregion

    namespace Random {
        //std::default_random_engine gGen;
        //
        //float Gen01();
        //
        //float Gen11();
    }

    namespace Math {
#include "Engine Includes/Types.h"

#ifndef PI
        static const float PI = 3.14159f;
#endif

#ifndef TAU
        static const float TAU = 2.f * PI;
#endif

        template<typename T>
        T sqr(T a) { return a * a; }

        template<typename T>
        T degtorad(T deg) { return deg * PI / 180.f; }

        template<typename T>
        T radtodeg(T rad) { return rad * 180.f / PI; }

        float point_distance(float2 from, float2 to);
        float point_distance(float3 from, float3 to);
        float point_distance(float4 from, float4 to);
        float point_distance(float2 to);
        float point_distance(float3 to);
        float point_distance(float4 to);

        // In deg
        float point_direction(float2 from, float2 to);
        float point_direction(float2 to);

        // In rad
        float point_direction_r(float2 from, float2 to);
        float point_direction_r(float2 to);

        // In deg
        //float2 point_direction(float3 from, float3 to);

        float lenghtdir_x(float len, float ang);
        float lenghtdir_y(float len, float ang);

        float dot(float2 a);
        float dot(float3 a);
        float dot(float4 a);
        float dot(float2 a, float2 b);
        float dot(float3 a, float3 b);
        float dot(float4 a, float4 b);

        template<typename T>
        T mad(T a, T b, T c) { return a * b + c; }

        template<typename T>
        T lerp(T from, T to, float coef) { return from + (to - from) * coef; }

        float3 cross(float3 a, float3 b);

        template<typename T>
        float length(T v) { return point_distance(v); }

        template<typename T>
        float length2(T v) { return dot(v, v); }

        float min(float a, float b);
        float2 min(float2 a, float b);
        float3 min(float3 a, float b);
        float4 min(float4 a, float b);
        float4 min(float4 a, float4 b);
        float3 min(float3 a, float3 b);
        float2 min(float2 a, float2 b);

        float max(float a, float b);
        float2 max(float2 a, float b);
        float3 max(float3 a, float b);
        float4 max(float4 a, float b);
        float4 max(float4 a, float4 b);
        float3 max(float3 a, float3 b);
        float2 max(float2 a, float2 b);

        float2 normalize(float2 v);
        float3 normalize(float3 v);
        float4 normalize(float4 v);

        template<typename T>
        T clamp(T v, T left, T right) { return max(min(v, right), left); }
        float3 clamp(float3 v, float left, float right);
        float4 clamp(float4 v, float left, float right);

        // Colors
        //float3 rgb2hsv(float3 rgb);
        //float3 hsv2hsv(float3 hsv);
        //float3 col_dim(float3 rgb, float value);
        //float col2float(float3 rgb, float a=1.f);
        //float col2float(float4 rgba);
        float3 saturate(float3 rgb);
        float4 saturate(float4 rgba);
        float4 rgb2rgba(float3 rgb, float a=1.f);
        float3 normrgb(float3 rgb);
        float4 normrgba(float4 rgba);
        //float3 srgb(float3 rgb);
        //float4 srgba(float4 rgba);

        // Quaternions


        // Matricies


        // 

    }

#pragma region Primitives
    namespace Draw {
#include "Engine Includes/Types.h"

        enum PrimitiveType {
            Noone, _Line, _Rectangle, _Circle, _Ellipse, _Triangle,
            _CircleOuter, _TriangleOuter, _RectangleOuter
        };

        struct PrimitiveColorBuffer {
            float4 _Color;
        };

        struct PrimitiveBuffer {
            float2 _PositionStart;
            float2 _PositionEnd;
            union {
                struct {
                    union {
                        struct { // Circle
                            float2 _Alignment_d;
                            float1 _Radius;
                        };

                        struct { // Ellipse
                            float2 _Radius2;
                            float1 _Alignment_c;
                        };
                    };

                    UINT _Vertices;
                };

                struct { // Triangle
                    float2 _Position3;
                    float2 _Alignment_b;
                };

                float4 _Alignment_a;
            };
        };

        struct MatrixBuffer {
            mfloat4x4 mWorld;
            mfloat4x4 mView;
            mfloat4x4 mProj;
            mfloat4 Params;
        };

        struct Config {
            float fNear = .2f, fFar = 2.f;
            float ViewW, ViewH;
            bool MSAA = false;
        };

        static Shader               *shLine;
        static Shader               *shCircle;
        static Shader               *shTriangle;
        static Shader               *shRectangle;
        static Shader               *shCircleOuter;
        static Shader               *shRectangleOuter;
        static Shader               *shTextureSimple;
        static Shader               *shTextureSimplePart;
        static PrimitiveType         gLastPrimitive = (PrimitiveType)0;
        static PrimitiveColorBuffer *gPrimColorBuff;
        static ConstantBuffer       *gPrimitiveColorBuff;
        static MatrixBuffer         *gMatrixBufferInst;
        static ConstantBuffer       *gPrimitiveBuffer;
        static ConstantBuffer       *gMatrixBuffer;
        static Config               *gConfig;

        void SetView (mfloat4x4 mView );
        void SetProj (mfloat4x4 mProj );
        void SetWorld(mfloat4x4 mWorld);

        void UpdateMatrixBuffer(mfloat4 vec={});

        void BindMatrixBuffer();

        void SetColor(float4 Color);

        void Resize(float W, float H);

        void Init(const Config& cfg);

        void Release();
        
        inline float  GetAlpha();
        inline float4 GetColor();

        void Line(float x1, float y1, float x2, float y2);

        void Rectangle(float x1, float y1, float x2, float y2);

        void Circle(float x, float y, float r, UINT precision=32);
        
        void Triangle(float x1, float y1, float x2, float y2, float x3, float y3);

        void CircleOuter(float x, float y, float r, UINT precision=32);

        void TextureRect(Texture* tex, float x, float y, float xscale=1.f, float yscale=1.f, float ang=0.f);
        void TextureStreched(Texture* tex, float x, float y, float xscale=1.f, float yscale=1.f, float ang=0.f);
    }
#pragma endregion

};

//#include "Utilities.inl"
