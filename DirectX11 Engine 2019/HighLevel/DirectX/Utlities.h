#pragma once

#include "Engine/DirectX/DirectX.h"
#include "Engine/Extensions/Safe.h"

#include <random>

extern _DirectX *gDirectX;

namespace LunaEngine {
    // Discard items(SRV, UAV, CB) from shader slots
#pragma region Compute Shader
    template<UINT dim>
    void CSDiscardUAV() {
        ID3D11UnorderedAccessView *pEmpty[dim] = { nullptr };
        gDirectX->gContext->CSSetUnorderedAccessViews(0, dim, pEmpty, 0);
    }

    template<>
    void CSDiscardUAV<1>() {
        ID3D11UnorderedAccessView *pEmpty = nullptr;
        gDirectX->gContext->CSSetUnorderedAccessViews(0, 1, &pEmpty, 0);
    }

    template<UINT dim>
    void CSDiscardCB() {
        ID3D11Buffer *pEmpty[dim] = nullptr;
        gDirectX->gContext->CSSetConstantBuffers(0, dim, pEmpty, 0);
    }

    template<>
    void CSDiscardCB<1>() {
        ID3D11Buffer *pEmpty = nullptr;
        gDirectX->gContext->CSSetConstantBuffers(0, 1, &pEmpty);
    }

    template<UINT dim>
    void CSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = nullptr;
        gDirectX->gContext->CSSetShaderResources(0, dim, pEmpty, 0);
    }

    template<>
    void CSDiscardSRV<1>() {
        ID3D11ShaderResourceView *pEmpty = nullptr;
        gDirectX->gContext->CSSetShaderResources(0, 1, &pEmpty);
    }
#pragma endregion

#pragma region Vertex Shader
    template<UINT dim>
    void VSDiscardCB() {
        ID3D11Buffer *pEmpty[dim] = nullptr;
        gDirectX->gContext->VSSetConstantBuffers(0, dim, pEmpty, 0);
    }

    template<>
    void VSDiscardCB<1>() {
        ID3D11Buffer *pEmpty = nullptr;
        gDirectX->gContext->VSSetConstantBuffers(0, 1, &pEmpty);
    }

    template<UINT dim>
    void VSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = nullptr;
        gDirectX->gContext->VSSetShaderResources(0, dim, pEmpty, 0);
    }

    template<>
    void VSDiscardSRV<1>() {
        ID3D11ShaderResourceView *pEmpty = nullptr;
        gDirectX->gContext->VSSetShaderResources(0, 1, &pEmpty);
    }
#pragma endregion

#pragma region Pixel Shader
    template<UINT dim>
    void PSDiscardCB() {
        ID3D11Buffer *pEmpty[dim] = nullptr;
        gDirectX->gContext->PSSetConstantBuffers(0, dim, pEmpty, 0);
    }

    template<>
    void PSDiscardCB<1>() {
        ID3D11Buffer *pEmpty = nullptr;
        gDirectX->gContext->PSSetConstantBuffers(0, 1, &pEmpty);
    }

    template<UINT dim>
    void PSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = nullptr;
        gDirectX->gContext->PSSetShaderResources(0, dim, pEmpty, 0);
    }

    template<>
    void PSDiscardSRV<1>() {
        ID3D11ShaderResourceView *pEmpty = nullptr;
        gDirectX->gContext->PSSetShaderResources(0, 1, &pEmpty);
    }
#pragma endregion

#pragma region Geometry Shader
    template<UINT dim>
    void GSDiscardCB() {
        ID3D11Buffer *pEmpty[dim] = nullptr;
        gDirectX->gContext->GSSetConstantBuffers(0, dim, pEmpty, 0);
    }

    template<>
    void GSDiscardCB<1>() {
        ID3D11Buffer *pEmpty = nullptr;
        gDirectX->gContext->GSSetConstantBuffers(0, 1, &pEmpty);
    }

    template<UINT dim>
    void GSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = nullptr;
        gDirectX->gContext->GSSetShaderResources(0, dim, pEmpty, 0);
    }

    template<>
    void GSDiscardSRV<1>() {
        ID3D11ShaderResourceView *pEmpty = nullptr;
        gDirectX->gContext->GSSetShaderResources(0, 1, &pEmpty);
    }
#pragma endregion

#pragma region Hull Shader
    template<UINT dim>
    void HSDiscardCB() {
        ID3D11Buffer *pEmpty[dim] = nullptr;
        gDirectX->gContext->HSSetConstantBuffers(0, dim, pEmpty, 0);
    }

    template<>
    void HSDiscardCB<1>() {
        ID3D11Buffer *pEmpty = nullptr;
        gDirectX->gContext->HSSetConstantBuffers(0, 1, &pEmpty);
    }

    template<UINT dim>
    void HSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = nullptr;
        gDirectX->gContext->HSSetShaderResources(0, dim, pEmpty, 0);
    }

    template<>
    void HSDiscardSRV<1>() {
        ID3D11ShaderResourceView *pEmpty = nullptr;
        gDirectX->gContext->HSSetShaderResources(0, 1, &pEmpty);
    }
#pragma endregion

#pragma region Domain Shader
    template<UINT dim>
    void DSDiscardCB() {
        ID3D11Buffer *pEmpty[dim] = nullptr;
        gDirectX->gContext->DSSetConstantBuffers(0, dim, pEmpty, 0);
    }

    template<>
    void DSDiscardCB<1>() {
        ID3D11Buffer *pEmpty = nullptr;
        gDirectX->gContext->DSSetConstantBuffers(0, 1, &pEmpty);
    }

    template<UINT dim>
    void DSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = nullptr;
        gDirectX->gContext->DSSetShaderResources(0, dim, pEmpty, 0);
    }

    template<>
    void DSDiscardSRV<1>() {
        ID3D11ShaderResourceView *pEmpty = nullptr;
        gDirectX->gContext->DSSetShaderResources(0, 1, &pEmpty);
    }
#pragma endregion
    
    namespace Random {
        std::default_random_engine gGen;

        float Gen01() {
            std::uniform_real_distribution<float> dist(0, 1);
            return dist(gGen);
        }

        float Gen11() {
            std::uniform_real_distribution<float> dist(1, 1);
            return dist(gGen);
        }
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
            _CircleOuter, 
        };

        struct PrimitiveColorBuffer {
            DirectX::XMFLOAT4 _Color;
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
        };

        struct Config {
            float fNear = .2f, fFar = 2.f;
            float ViewW, ViewH;
        };

        static Shader               *shCircleOuter;
        static Shader               *shLine;
        static Shader               *shCircle;
        static Shader               *shTriangle;
        static Shader               *shRectangle;
        static PrimitiveType         gLastPrimitive = PrimitiveType::Noone;
        static PrimitiveColorBuffer *gPrimColorBuff;
        static ConstantBuffer       *gPrimitiveColorBuff;
        static MatrixBuffer         *gMatrixBufferInst;
        static ConstantBuffer       *gPrimitiveBuffer;
        static ConstantBuffer       *gMatrixBuffer;
        static Config               *gConfig;

        void SetView (mfloat4x4 mView ) { gMatrixBufferInst->mView  = mView;  }
        void SetProj (mfloat4x4 mProj ) { gMatrixBufferInst->mProj  = mProj;  }
        void SetWorld(mfloat4x4 mWorld) { gMatrixBufferInst->mWorld = mWorld; }

        void UpdateMatrixBuffer() {
            MatrixBuffer *tmp = (MatrixBuffer*)gMatrixBuffer->Map();
                tmp->mView  = gMatrixBufferInst->mView;
                tmp->mProj  = gMatrixBufferInst->mProj;
                tmp->mWorld = gMatrixBufferInst->mWorld;
            gMatrixBuffer->Unmap();
        }

        void BindMatrixBuffer() { gMatrixBuffer->Bind(Shader::Vertex, 0); }

        void SetColor(float4 Color) {
            PrimitiveColorBuffer *pcBuff = (PrimitiveColorBuffer*)gPrimitiveColorBuff->Map();
                pcBuff->_Color = Color;
            gPrimitiveColorBuff->Unmap();

            // 
            gPrimColorBuff->_Color = Color;
        }

        void Resize(float W, float H) {
            gConfig->ViewW = W;
            gConfig->ViewH = H;

            gMatrixBufferInst->mProj =
                DirectX::XMMatrixOrthographicOffCenterLH(0.f, gConfig->ViewW, gConfig->ViewH,
                                                         0.f, gConfig->fNear, gConfig->fFar);
        }

        void Init(const Config& cfg) {
            // 
            gConfig = new Config();
            gConfig->fFar  = cfg.fFar;
            gConfig->fNear = cfg.fNear;

            // Create long live instances
            shLine = new Shader();
            shLine->LoadFile("shLineVS.cso", Shader::Vertex);
            shLine->LoadFile("shLinePS.cso", Shader::Pixel);

            shCircle = new Shader();
            shCircle->LoadFile("shCircleVS.cso", Shader::Vertex);
            shCircle->AttachShader(shLine, Shader::Pixel);

            shTriangle = new Shader();
            shTriangle->LoadFile("shTriangleVS.cso", Shader::Vertex);
            shTriangle->AttachShader(shLine, Shader::Pixel);

            shRectangle = new Shader();
            shRectangle->LoadFile("shRectangleVS.cso", Shader::Vertex);
            shRectangle->AttachShader(shLine, Shader::Pixel);

            shCircleOuter = new Shader();
            shCircleOuter->LoadFile("shCircleOuterVS.cso", Shader::Vertex);
            shCircleOuter->AttachShader(shLine, Shader::Pixel);

            shCircleOuter->ReleaseBlobs();
            shRectangle->ReleaseBlobs();
            shTriangle->ReleaseBlobs();
            shCircle->ReleaseBlobs();
            shLine->ReleaseBlobs();

            // Create constant buffers
            gPrimitiveColorBuff = new ConstantBuffer(sizeof(PrimitiveColorBuffer));
            gPrimitiveBuffer    = new ConstantBuffer(sizeof(PrimitiveBuffer));
            gMatrixBuffer       = new ConstantBuffer(sizeof(MatrixBuffer));
            
            // Set names
            gPrimitiveColorBuff->SetName("PSB");
            gPrimitiveBuffer->SetName("PDB");
            gMatrixBuffer->SetName("PMB");

            // Create heap buffers
            gMatrixBufferInst = new MatrixBuffer();
            gPrimColorBuff    = new PrimitiveColorBuffer();

            // Update matrices
            Resize(cfg.ViewW, cfg.ViewH);
            SetView(DirectX::XMMatrixIdentity());
            SetWorld(DirectX::XMMatrixIdentity());

            UpdateMatrixBuffer();

            // Set default color
            SetColor({ 1.f, 1.f, 1.f, 1.f });
        }

        void Release() {
            SAFE_RELEASE(shCircleOuter);
            SAFE_RELEASE(shRectangle  );
            SAFE_RELEASE(shTriangle   );
            SAFE_RELEASE(shCircle     );
            SAFE_RELEASE(shLine       );

            SAFE_RELEASE(gPrimitiveColorBuff);
            SAFE_RELEASE(gPrimitiveBuffer   );
            SAFE_RELEASE(gMatrixBuffer      );

            SAFE_DELETE(gConfig          );
            SAFE_DELETE(gMatrixBufferInst);
            SAFE_DELETE(gPrimColorBuff   );
        }

        inline float  GetAlpha() { return gPrimColorBuff->_Color.w; }
        inline float4 GetColor() { return gPrimColorBuff->_Color; }

        void Line(float x1, float y1, float x2, float y2) {
            if( Shader::GetBound() != shLine || gLastPrimitive != PrimitiveType::_Line ) {
                gLastPrimitive = PrimitiveType::_Line;                                         // Update state
                shLine->Bind();                                                                // Set shader
                gDirectX->gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST); // Set topology
            }

            // Update primitive buffer
            PrimitiveBuffer *pBuff = (PrimitiveBuffer*)gPrimitiveBuffer->Map();
                pBuff->_PositionStart = { x1, y1 };
                pBuff->_PositionEnd   = { x2, y2 };
            gPrimitiveBuffer->Unmap();

            // Bind Buffers
            BindMatrixBuffer();
            gPrimitiveBuffer->Bind(Shader::Vertex, 1);
            gPrimitiveColorBuff->Bind(Shader::Pixel, 0);

            // Draw call
            gDirectX->gContext->Draw(2, 0);
        }

        void Rectangle(float x1, float y1, float x2, float y2) {
            if( Shader::GetBound() != shRectangle || gLastPrimitive != PrimitiveType::_Rectangle ) {
                gLastPrimitive = PrimitiveType::_Rectangle;                                        // Update state
                shRectangle->Bind();                                                               // Set shader
                gDirectX->gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // Set topology
            }

            // Update primitive buffer
            PrimitiveBuffer *pBuff = (PrimitiveBuffer*)gPrimitiveBuffer->Map();
            pBuff->_PositionStart = { x1, y1 };
            pBuff->_PositionEnd = { x2, y2 };
            gPrimitiveBuffer->Unmap();

            // Bind Buffers
            BindMatrixBuffer();
            gPrimitiveBuffer->Bind(Shader::Vertex, 1);
            gPrimitiveColorBuff->Bind(Shader::Pixel, 0);

            // Draw call
            gDirectX->gContext->Draw(6, 0);
        }

        void Circle(float x, float y, float r, UINT precision=32) {
            if( Shader::GetBound() != shCircle || gLastPrimitive != PrimitiveType::_Circle ) {
                gLastPrimitive = PrimitiveType::_Circle;                                           // Update state
                shCircle->Bind();                                                                  // Set shader
                gDirectX->gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // Set topology
            }

            // Update primitive buffer
            PrimitiveBuffer *pBuff = (PrimitiveBuffer*)gPrimitiveBuffer->Map();
                pBuff->_PositionStart = { x, y };
                pBuff->_Radius        = r;
                pBuff->_Vertices      = precision;
            gPrimitiveBuffer->Unmap();

            // Bind Buffers
            BindMatrixBuffer();
            gPrimitiveBuffer->Bind(Shader::Vertex, 1);
            gPrimitiveColorBuff->Bind(Shader::Pixel, 0);

            // Draw call
            gDirectX->gContext->Draw(precision * 3 + 1, 0);
        }
        
        void Triangle(float x1, float y1, float x2, float y2, float x3, float y3) {
            if( Shader::GetBound() != shTriangle || gLastPrimitive != PrimitiveType::_Triangle ) {
                gLastPrimitive = PrimitiveType::_Triangle;                                         // Update state
                shTriangle->Bind();                                                                // Set shader
                gDirectX->gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // Set topology
            }

            // Update primitive buffer
            PrimitiveBuffer *pBuff = (PrimitiveBuffer*)gPrimitiveBuffer->Map();
                pBuff->_PositionStart = { x1, y1 };
                pBuff->_PositionEnd   = { x2, y2 };
                pBuff->_Position3     = { x3, y3 };
            gPrimitiveBuffer->Unmap();

            // Bind Buffers
            BindMatrixBuffer();
            gPrimitiveBuffer->Bind(Shader::Vertex, 1);
            gPrimitiveColorBuff->Bind(Shader::Pixel, 0);

            // Draw call
            gDirectX->gContext->Draw(3, 0);
        }

        void CircleOuter(float x, float y, float r, UINT precision=32) {
            if( Shader::GetBound() != shCircleOuter || gLastPrimitive != PrimitiveType::_CircleOuter ) {
                gLastPrimitive = PrimitiveType::_CircleOuter;                                   // Update state
                shCircleOuter->Bind();                                                          // Set shader
                gDirectX->gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP); // Set topology
            }

            // Update primitive buffer
            PrimitiveBuffer *pBuff = (PrimitiveBuffer*)gPrimitiveBuffer->Map();
                pBuff->_PositionStart = { x, y };
                pBuff->_Radius        = r;
                pBuff->_Vertices      = precision;
            gPrimitiveBuffer->Unmap();

            // Bind Buffers
            BindMatrixBuffer();
            gPrimitiveBuffer->Bind(Shader::Vertex, 1);
            gPrimitiveColorBuff->Bind(Shader::Pixel, 0);

            // Draw call
            gDirectX->gContext->Draw(precision + 1, 0);
        }


    }
#pragma endregion

};
