#pragma once

#include "Engine/Extensions/Safe.h"
#include "Engine/Utility/Timer.h"
#include "Engine/Profiler/ScopedRangeProfiler.h"
#include "Engine Includes/Core.h"
#include "Engine/Materials/Texture.h"

//#include <random>
#undef min
#undef max

extern _DirectX *gDirectX;

namespace LunaEngine {
    // Discard items(SRV, UAV, CB) from shader slots
#pragma region Compute Shader
    template<UINT dim>
    void CSDiscardUAV();

    template<>
    void CSDiscardUAV<1>();

    template<UINT dim>
    void CSDiscardCB();

    template<>
    void CSDiscardCB<1>();

    template<UINT dim>
    void CSDiscardSRV();

    template<>
    void CSDiscardSRV<1>();
#pragma endregion

#pragma region Vertex Shader
    template<UINT dim>
    void VSDiscardCB();

    template<>
    void VSDiscardCB<1>();

    template<UINT dim>
    void VSDiscardSRV();

    template<>
    void VSDiscardSRV<1>();
#pragma endregion

#pragma region Pixel Shader
    template<UINT dim>
    void PSDiscardCB();

    template<>
    void PSDiscardCB<1>();

    template<UINT dim>
    void PSDiscardSRV();

    template<>
    void PSDiscardSRV<1>();
#pragma endregion

#pragma region Geometry Shader
    template<UINT dim>
    void GSDiscardCB();

    template<>
    void GSDiscardCB<1>();

    template<UINT dim>
    void GSDiscardSRV();

    template<>
    void GSDiscardSRV<1>();
#pragma endregion

#pragma region Hull Shader
    template<UINT dim>
    void HSDiscardCB();

    template<>
    void HSDiscardCB<1>();

    template<UINT dim>
    void HSDiscardSRV();

    template<>
    void HSDiscardSRV<1>();
#pragma endregion

#pragma region Domain Shader
    template<UINT dim>
    void DSDiscardCB();

    template<>
    void DSDiscardCB<1>();

    template<UINT dim>
    void DSDiscardSRV();

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
        T sqr(T a);

        template<typename T>
        T degtorad(T deg);

        template<typename T>
        T radtodeg(T rad);

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
        T mad(T a, T b, T c);

        template<typename T>
        T lerp(T from, T to, float coef);

        float3 cross(float3 a, float3 b);

#ifndef min
        template<typename T>
        T min(T a, T b);
#endif

#ifndef max
        template<typename T>
        T max(T a, T b);
#endif

        template<typename T>
        T clamp(T v, T left, T right);
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
        typedef float               float1;
        typedef DirectX::XMFLOAT2   float2;
        typedef DirectX::XMFLOAT3   float3;
        typedef DirectX::XMFLOAT4   float4;
        typedef DirectX::XMVECTOR   vfloat;
        typedef DirectX::XMFLOAT4X4 float4x4;
        typedef DirectX::XMMATRIX   mfloat4x4;

        enum PrimitiveType {
            Noone, _Line, _Rectangle, _Circle, _Ellipse, _Triangle,
            _CircleOuter, _TriangleOuter, _RectangleOuter
        };


        struct PrimitiveColorBuffer;

        struct PrimitiveBuffer;

        struct MatrixBuffer;

        struct Config;

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

#include "Utilities.inl"
