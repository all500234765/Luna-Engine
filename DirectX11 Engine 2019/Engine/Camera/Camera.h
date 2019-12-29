#pragma once

#include "pc.h"
#include "Engine/DirectX/ConstantBuffer.h"
#include "Engine/States/PipelineState.h"
#include "Engine Includes/Types.h"

struct CameraConfig {
    float FOV /* X */, fAspect, fNear, fFar;
    bool Ortho=false;
    float ViewW, ViewH;
};

class Camera: public PipelineState<Camera> {
private:
    float3 pPos, pRot;
    mfloat4x4 mProj, mView, mWorld;
    ConstantBuffer *cb;

    CameraConfig cfg;

    // Must keep the same order in the shader
    struct BufferMatrix {
        mfloat4x4 mWorld;
        mfloat4x4 mView;
        mfloat4x4 mProj;
        mfloat4 vPosition;
    };

public:
    Camera();
    Camera(float3 p);
    Camera(float3 p, float3 r);

    void Init(); // Internal

    // 
    inline void Bind() { gState = this; };

    inline void SetParams(CameraConfig config) { cfg = config; };
    inline CameraConfig GetParams() const { return cfg; };

    void BuildView();
    void BuildProj();

    void Translate(float3 p);
    void TranslateAbs(float3 p);
    void TranslateLookAt(float3 p);
    void TranslateLookAtAbs(float3 p);

    void Rotate(float3 r);
    void RotateAbs(float3 r);

    const ConstantBuffer& BuildConstantBuffer(float wparam=1.f);
    const ConstantBuffer& BuildConstantBuffer(mfloat4 wparam);
    void BindBuffer(UINT type, UINT slot);
    void BindBuffer(Shader::ShaderType type, UINT slot) { BindBuffer(static_cast<UINT>(type), slot); }

    inline float3 GetPosition() const { return pPos; };
    inline void SetPosition(float3 p) { pPos = p; };

    inline float3 GetRotation() const { return pRot; }
    inline void SetRotation(float3 r) { pRot = r; };

    inline void SetWorldMatrix(mfloat4x4 w) { mWorld = w; }
    inline mfloat4x4 GetWorldMatrix() const { return mWorld; }

    inline void SetViewMatrix(mfloat4x4 view) { mView = view; }
    inline mfloat4x4 GetViewMatrix() const { return mView; }

    inline void SetProjMatrix(mfloat4x4 proj) { mProj = proj; }
    inline mfloat4x4 GetProjMatrix() const { return mProj; }

    inline float GetAspect() const { return cfg.fAspect; }
    inline float GetNear() const { return cfg.fNear; }
    inline float GetFar() const { return cfg.fFar; }

    // TODO: Store inverse view matrix (world matrix for camera) 1st
    //inline float3 GetRight() const { return reinterpret_cast<const DirectX::XMFLOAT3*>(&mCameraWorld._11); }
    //inline float3 GetUp() const { return reinterpret_cast<const DirectX::XMFLOAT3*>(&mCameraWorld._21); }
    //inline float3 GetForward() const { return reinterpret_cast<const DirectX::XMFLOAT3*>(&mCameraWorld._31); }
};
