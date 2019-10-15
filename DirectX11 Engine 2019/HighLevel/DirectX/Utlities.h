#pragma once

#include "Engine/Extensions/Safe.h"
#include "Engine/Utility/Timer.h"
#include "Engine Includes/Core.h"

//#include <random>

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

#pragma region Primitives
    namespace Draw {
        typedef float               float1;
        typedef DirectX::XMFLOAT2   float2;
        typedef DirectX::XMFLOAT3   float3;
        typedef DirectX::XMFLOAT4   float4;
        typedef DirectX::XMVECTOR   vfloat;
        typedef DirectX::XMFLOAT4X4 float4x4;
        typedef DirectX::XMMATRIX   mfloat4x4;

        enum PrimitiveType;

        struct PrimitiveColorBuffer;

        struct PrimitiveBuffer;

        struct MatrixBuffer;

        struct Config;

        static Shader               *shCircleOuter;
        static Shader               *shLine;
        static Shader               *shCircle;
        static Shader               *shTriangle;
        static Shader               *shRectangle;
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

        void UpdateMatrixBuffer();

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


    }
#pragma endregion

};

#include "Utilities.inl"
