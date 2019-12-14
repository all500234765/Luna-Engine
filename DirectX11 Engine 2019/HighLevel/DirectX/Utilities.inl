//#include "Utilities.h"
#pragma once

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
        ID3D11Buffer *pEmpty[dim] = { nullptr };
        gDirectX->gContext->CSSetConstantBuffers(0, dim, pEmpty);
    }

    template<>
    void CSDiscardCB<1>() {
        ID3D11Buffer *pEmpty = nullptr;
        gDirectX->gContext->CSSetConstantBuffers(0, 1, &pEmpty);
    }

    template<UINT dim>
    void CSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = { nullptr };
        gDirectX->gContext->CSSetShaderResources(0, dim, pEmpty);
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
        ID3D11Buffer *pEmpty[dim] = { nullptr };
        gDirectX->gContext->VSSetConstantBuffers(0, dim, pEmpty);
    }

    template<>
    void VSDiscardCB<1>() {
        ID3D11Buffer *pEmpty = nullptr;
        gDirectX->gContext->VSSetConstantBuffers(0, 1, &pEmpty);
    }

    template<UINT dim>
    void VSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = { nullptr };
        gDirectX->gContext->VSSetShaderResources(0, dim, pEmpty);
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
        ID3D11Buffer *pEmpty[dim] = { nullptr };
        gDirectX->gContext->PSSetConstantBuffers(0, dim, pEmpty);
    }

    template<>
    void PSDiscardCB<1>() {
        ID3D11Buffer *pEmpty = nullptr;
        gDirectX->gContext->PSSetConstantBuffers(0, 1, &pEmpty);
    }

    template<UINT dim>
    void PSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = { nullptr };
        gDirectX->gContext->PSSetShaderResources(0, dim, pEmpty);
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
        ID3D11Buffer *pEmpty[dim] = { nullptr };
        gDirectX->gContext->GSSetConstantBuffers(0, dim, pEmpty);
    }

    template<>
    void GSDiscardCB<1>() {
        ID3D11Buffer *pEmpty = nullptr;
        gDirectX->gContext->GSSetConstantBuffers(0, 1, &pEmpty);
    }

    template<UINT dim>
    void GSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = { nullptr };
        gDirectX->gContext->GSSetShaderResources(0, dim, pEmpty);
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
        ID3D11Buffer *pEmpty[dim] = { nullptr };
        gDirectX->gContext->HSSetConstantBuffers(0, dim, pEmpty);
    }

    template<>
    void HSDiscardCB<1>() {
        ID3D11Buffer *pEmpty = nullptr;
        gDirectX->gContext->HSSetConstantBuffers(0, 1, &pEmpty);
    }

    template<UINT dim>
    void HSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = { nullptr };
        gDirectX->gContext->HSSetShaderResources(0, dim, pEmpty);
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
        ID3D11Buffer *pEmpty[dim] = { nullptr };
        gDirectX->gContext->DSSetConstantBuffers(0, dim, pEmpty);
    }

    template<>
    void DSDiscardCB<1>() {
        ID3D11Buffer *pEmpty = nullptr;
        gDirectX->gContext->DSSetConstantBuffers(0, 1, &pEmpty);
    }

    template<UINT dim>
    void DSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = { nullptr };
        gDirectX->gContext->DSSetShaderResources(0, dim, pEmpty);
    }

    template<>
    void DSDiscardSRV<1>() {
        ID3D11ShaderResourceView *pEmpty = nullptr;
        gDirectX->gContext->DSSetShaderResources(0, 1, &pEmpty);
    }
#pragma endregion

    namespace Random {
        /*float Gen01() {
            std::uniform_real_distribution<float> dist(0, 1);
            return dist(gGen);
        }

        float Random::Gen11() {
            std::uniform_real_distribution<float> dist(1, 1);
            return dist(gGen);
        }*/
    }

    namespace Math {

        template<typename T>
        T sqr(T a) { return a * a; }

        template<typename T>
        T degtorad(T deg) { return deg * PI / 180.f; }

        template<typename T>
        T radtodeg(T rad) { return rad * 180.f / PI; }

        float point_distance(float2 from, float2 to) { return sqrtf(sqr(to.x - from.x) + sqr(to.y - from.y)); }
        float point_distance(float3 from, float3 to) { return sqrtf(sqr(to.x - from.x) + sqr(to.y - from.y) + sqr(to.z - from.z)); }
        float point_distance(float4 from, float4 to) { return sqrtf(sqr(to.x - from.x) + sqr(to.y - from.y) + sqr(to.z - from.z) + sqr(to.w - from.w)); }
        float point_distance(float2 to) { return sqrtf(sqr(to.x) + sqr(to.y)); }
        float point_distance(float3 to) { return sqrtf(sqr(to.x) + sqr(to.y) + sqr(to.z)); }
        float point_distance(float4 to) { return sqrtf(sqr(to.x) + sqr(to.y) + sqr(to.z) + sqr(to.w)); }

        // In deg
        float point_direction  (float2 from, float2 to) { return radtodeg(atan2f(from.y - to.y, to.x - from.x)); }
        float point_direction  (float2 to             ) { return radtodeg(atan2f(       - to.y, to.x         )); }

        // In rad
        float point_direction_r(float2 from, float2 to) { return         (atan2f(from.y - to.y, to.x - from.x)); }
        float point_direction_r(float2 to             ) { return         (atan2f(       - to.y, to.x         )); }

        //float2 point_direction(float3 from, float3 to);

        float lenghtdir_x(float len, float deg) { return len * radtodeg(cosf(deg)); }
        float lenghtdir_y(float len, float deg) { return len * radtodeg(sinf(deg)); }

        float dot(float2 a) { return sqr(a.x) + sqr(a.y); }
        float dot(float3 a) { return sqr(a.x) + sqr(a.y) + sqr(a.z); }
        float dot(float4 a) { return sqr(a.x) + sqr(a.y) + sqr(a.z) + sqr(a.w); }
        float dot(float2 a, float2 b) { return a.x * b.x + a.y * b.y; }
        float dot(float3 a, float3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
        float dot(float4 a, float4 b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

        template<typename T>
        T mad(T a, T b, T c) { return a * b + c; }

        template<typename T>
        T lerp(T from, T to, float coef) { return from + (to - from) * coef; }

        float3 cross(float3 a, float3 b) {
            return {
                a.y * b.z - a.z * b.y, 
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x
            };
        }

        template<typename T>
        float length(T v) { return point_distance(v); }

        float4 normalize(float4 v) {
            if( v.x == 0.f && v.y == 0.f && v.z == 0.f && v.w == 0.f ) return { 0.f, 0.f, 1.f, 0.f };

            float l = length(v);
            return { v.x / l, v.y / l, v.z / l, v.w / l };
        }

        float3 normalize(float3 v) {
            if( v.x == 0.f && v.y == 0.f && v.z == 0.f ) return { 0.f, 0.f, 1.f };

            float l = length(v);
            return { v.x / l, v.y / l, v.z / l };
        }

#ifndef min
        template<typename T>
        T min(T a, T b) { return (a < b) ? a : b; };
#endif

#ifndef max
        template<typename T>
        T max(T a, T b) { return (a > b) ? a : b; };
#endif

        template<typename T>
        T clamp(T v, T left, T right) { return max(min(v, right), left); }
        float3 clamp(float3 v, float left, float right) { return { clamp(v.x, left, right), clamp(v.y, left, right), clamp(v.z, left, right) }; }
        float4 clamp(float4 v, float left, float right) { return { clamp(v.x, left, right), clamp(v.y, left, right), clamp(v.z, left, right), clamp(v.w, left, right) }; }

        // Colors
        //float3 rgb2hsv(float3 rgb);
        //float3 hsv2hsv(float3 hsv);
        //float3 col_dim(float3 rgb, float value) {
        //    float3 hsv = rgb2hsv(rgb);
        //    hsv.b = value;
        //    return hsv2rgb(hsv);
        //}
        //float col2float(float3 rgb, float a);
        //float col2float(float4 rgba);
        float3 saturate(float3 rgb)  { return clamp(rgb, 0.f, 1.f); }
        float4 saturate(float4 rgba) { return clamp(rgba, 0.f, 1.f); }
        float4 rgb2rgba(float3 rgb, float a) { return { rgb.x, rgb.y, rgb.z, a }; }
        float3 normrgb( float3 rgb ) { return { rgb.x / 255.f, rgb.y / 255.f, rgb.z / 255.f }; }
        float4 normrgba(float4 rgba) { return { rgba.x / 255.f, rgba.y / 255.f, rgba.z / 255.f, rgba.w / 255.f }; }

        // Quaternions


        // Matricies


        // 

    }



#pragma region Primitives
    namespace Draw {
#include "Engine Includes/Types.h"

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

        void SetView(mfloat4x4 mView) { gMatrixBufferInst->mView = mView; }
        void SetProj(mfloat4x4 mProj) { gMatrixBufferInst->mProj = mProj; }
        void SetWorld(mfloat4x4 mWorld) { gMatrixBufferInst->mWorld = mWorld; }

        void UpdateMatrixBuffer(mfloat4 vec) {
            MatrixBuffer *tmp = (MatrixBuffer*)gMatrixBuffer->Map();
                tmp->mView = gMatrixBufferInst->mView;
                tmp->mProj = gMatrixBufferInst->mProj;
                tmp->mWorld = gMatrixBufferInst->mWorld;
                tmp->Params = vec;
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

            shRectangleOuter = new Shader();
            shRectangleOuter->LoadFile("shRectangleOuterVS.cso", Shader::Vertex);
            shRectangleOuter->AttachShader(shLine, Shader::Pixel);

            shTextureSimple = new Shader();
            shTextureSimple->LoadFile("shTexturedQuadAutoVS.cso", Shader::Vertex);
            shTextureSimple->LoadFile("shTexturedQuadNoFXPS.cso", Shader::Pixel );

            shTextureSimplePart = new Shader();
            shTextureSimplePart->LoadFile("shTexturedQuadPartVS.cso", Shader::Vertex);
            shTextureSimplePart->AttachShader(shTextureSimple, Shader::Pixel);

            shTextureSimplePart->ReleaseBlobs();
            shTextureSimple->ReleaseBlobs();
            shRectangleOuter->ReleaseBlobs();
            shCircleOuter->ReleaseBlobs();
            shRectangle->ReleaseBlobs();
            shTriangle->ReleaseBlobs();
            shCircle->ReleaseBlobs();
            shLine->ReleaseBlobs();

            // Reload shader
            // TODO: Add function to update Config and reload shaders if needed
            /*if( cfg.MSAA ) {
                shTextureSimple->Reload("shTexturedQuadNoFXPSMSAA.cso", Shader::Pixel);
                shTextureSimple->ReleaseBlobs();
            }*/

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
            SAFE_RELEASE(shTextureSimplePart);
            SAFE_RELEASE(shTextureSimple    );
            SAFE_RELEASE(shRectangleOuter   );
            SAFE_RELEASE(shCircleOuter      );
            SAFE_RELEASE(shRectangle        );
            SAFE_RELEASE(shTriangle         );
            SAFE_RELEASE(shCircle           );
            SAFE_RELEASE(shLine             );

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
            ScopedRangeProfiler e0(__FUNCTIONW__);
            if( Shader::Current() != shLine || gLastPrimitive != PrimitiveType::_Line ) {
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
            ScopedRangeProfiler e0(__FUNCTIONW__);
            if( Shader::Current() != shRectangle || gLastPrimitive != PrimitiveType::_Rectangle ) {
                gLastPrimitive = PrimitiveType::_Rectangle;                                        // Update state
                shRectangle->Bind();                                                               // Set shader
                gDirectX->gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // Set topology
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
            gDirectX->gContext->Draw(6, 0);
        }

        void Circle(float x, float y, float r, UINT precision) {
            ScopedRangeProfiler e0(__FUNCTIONW__);
            if( Shader::Current() != shCircle || gLastPrimitive != PrimitiveType::_Circle ) {
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
            ScopedRangeProfiler e0(__FUNCTIONW__);
            if( Shader::Current() != shTriangle || gLastPrimitive != PrimitiveType::_Triangle ) {
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
        
        
        void RectangleOuter(float x1, float y1, float x2, float y2) {
            ScopedRangeProfiler e0(__FUNCTIONW__);
            if( Shader::Current() != shRectangleOuter || gLastPrimitive != PrimitiveType::_RectangleOuter ) {
                gLastPrimitive = PrimitiveType::_RectangleOuter;                                   // Update state
                shRectangleOuter->Bind();                                                          // Set shader
                gDirectX->gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);    // Set topology
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
            gDirectX->gContext->Draw(5, 0);
        }

        void TriangleOuter(float x1, float y1, float x2, float y2, float x3, float y3) {
            ScopedRangeProfiler e0(__FUNCTIONW__);
            if( Shader::Current() != shTriangle || gLastPrimitive != PrimitiveType::_TriangleOuter ) {
                gLastPrimitive = PrimitiveType::_TriangleOuter;                                 // Update state
                shTriangle->Bind();                                                             // Set shader
                gDirectX->gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP); // Set topology
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
            gDirectX->gContext->Draw(4, 0);
        }

        void CircleOuter(float x, float y, float r, UINT precision) {
            ScopedRangeProfiler e0(__FUNCTIONW__);
            if( Shader::Current() != shCircleOuter || gLastPrimitive != PrimitiveType::_CircleOuter ) {
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


        void TextureRect(Texture* tex, float x, float y, float xscale, float yscale, float ang) {
            ScopedRangeProfiler s0(__FUNCTIONW__);
            if( Shader::Current() != shTextureSimple ) {
                shTextureSimple->Bind();                                                           // Set shader
                gDirectX->gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // Set topology
            }

            float w = tex->GetWidth();
            float h = tex->GetHeight();

            // Build world matrix & Update cbuffer
            mfloat4x4 mWorld = DirectX::XMMatrixIdentity();
            mWorld *= DirectX::XMMatrixScaling(w * xscale, -h * yscale, 1.f);
            //mWorld *= DirectX::XMMatrixTranslation(-.5f, .5f, 0.f);
            mWorld *= DirectX::XMMatrixRotationZ(Math::degtorad(ang));
            //mWorld *= DirectX::XMMatrixTranslation(.5f, -.5f, 0.f);
            mWorld *= DirectX::XMMatrixTranslation(x + w * xscale, y + h * yscale, 0.f);

            SetWorld(mWorld);
            UpdateMatrixBuffer();

            // Bind resources
            BindMatrixBuffer();

            tex->Bind(Shader::Pixel, 0);

            // Draw call
            gDirectX->gContext->Draw(6, 0);
        }

        void TextureStreched(Texture* tex, float x, float y, float xscale, float yscale, float ang) {
            Draw::TextureRect(tex, 0.f, 0.f, xscale / (2.f * tex->GetWidth()), yscale / (2.f * tex->GetHeight()), ang);
        }

        void TexturePartScaled(Texture* tex, float x, float y, float left, float top, 
                                 float width, float height, float xscale, float yscale) {
            ScopedRangeProfiler s0(__FUNCTIONW__);
            if( Shader::Current() != shTextureSimplePart ) {
                shTextureSimplePart->Bind();                                                       // Set shader
                gDirectX->gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // Set topology
            }

            float w = tex->GetWidth();
            float h = tex->GetHeight();

            // Scale down
            xscale /= w * 2.f;
            yscale /= h * 2.f;

            // Build world matrix & Update cbuffer
            mfloat4x4 mWorld = DirectX::XMMatrixIdentity();
            mWorld *= DirectX::XMMatrixScaling(w * xscale, -h * yscale, 1.f);
            mWorld *= DirectX::XMMatrixTranslation(x + w * xscale, y + h * yscale, 0.f);

            SetWorld(mWorld);
            UpdateMatrixBuffer({ left / w, top / h, width / w, height / h });

            // Bind resources
            BindMatrixBuffer();

            tex->Bind(Shader::Pixel, 0);

            // Draw call
            gDirectX->gContext->Draw(6, 0);
        }
    }

}
