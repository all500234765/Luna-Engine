#pragma once

#include <DirectXMath.h>
#include "Engine/DirectX/ConstantBuffer.h"
#include "Engine/States/PipelineState.h"

struct CameraConfig {
    float FOV /* X */, fAspect, fNear, fFar;
    bool Ortho=false;
    float ViewW, ViewH;
};

class Camera: public PipelineState<Camera> {
private:
    DirectX::XMFLOAT3 pPos, pRot;
    DirectX::XMMATRIX mProj, mView, mWorld;
    ConstantBuffer *cb;

    CameraConfig cfg;

    // Must keep the same order in the shader
    struct BufferMatrix {
        DirectX::XMMATRIX mWorld;
        DirectX::XMMATRIX mView;
        DirectX::XMMATRIX mProj;
        DirectX::XMVECTOR vPosition;
    };

public:
    Camera();
    Camera(DirectX::XMFLOAT3 p);
    Camera(DirectX::XMFLOAT3 p, DirectX::XMFLOAT3 r);

    void Init(); // Internal

    // 
    inline void Bind() { gState = this; };

    inline void SetParams(CameraConfig config) { cfg = config; };
    inline CameraConfig GetParams() const { return cfg; };

    void BuildView();
    void BuildProj();

    void Translate(DirectX::XMFLOAT3 p);
    void TranslateAbs(DirectX::XMFLOAT3 p);
    void TranslateLookAt(DirectX::XMFLOAT3 p);
    void TranslateLookAtAbs(DirectX::XMFLOAT3 p);

    void Rotate(DirectX::XMFLOAT3 r);
    void RotateAbs(DirectX::XMFLOAT3 r);

    const ConstantBuffer& BuildConstantBuffer(float wparam=1.f);
    const ConstantBuffer& BuildConstantBuffer(DirectX::XMVECTOR wparam);
    void BindBuffer(UINT type, UINT slot);
    void BindBuffer(Shader::ShaderType type, UINT slot) { BindBuffer(static_cast<UINT>(type), slot); }

    inline DirectX::XMFLOAT3 GetPosition() const { return pPos; };
    inline void SetPosition(DirectX::XMFLOAT3 p) { pPos = p; };

    inline DirectX::XMFLOAT3 GetRotation() const { return pRot; }
    inline void SetRotation(DirectX::XMFLOAT3 r) { pRot = r; };

    inline void SetWorldMatrix(DirectX::XMMATRIX w) { mWorld = w; }
    inline DirectX::XMMATRIX GetWorldMatrix() const { return mWorld; }

    inline void SetViewMatrix(DirectX::XMMATRIX view) { mView = view; }
    inline DirectX::XMMATRIX GetViewMatrix() const { return mView; }

    inline void SetProjMatrix(DirectX::XMMATRIX proj) { mProj = proj; }
    inline DirectX::XMMATRIX GetProjMatrix() const { return mProj; }

    inline float GetAspect() const { return cfg.fAspect; }
    inline float GetNear() const { return cfg.fNear; }
    inline float GetFar() const { return cfg.fFar; }

    // TODO: Store inverse view matrix (world matrix for camera) 1st
    //inline float3 GetRight() const { return reinterpret_cast<const DirectX::XMFLOAT3*>(&mCameraWorld._11); }
    //inline float3 GetUp() const { return reinterpret_cast<const DirectX::XMFLOAT3*>(&mCameraWorld._21); }
    //inline float3 GetForward() const { return reinterpret_cast<const DirectX::XMFLOAT3*>(&mCameraWorld._31); }
};
