#pragma once

#include <DirectXMath.h>
#include "Engine/DirectX/ConstantBuffer.h"

struct CameraConfig {
    float FOV /* X */, fAspect, fNear, fFar;
    bool Ortho=false;
    float ViewW, ViewH;
};

class Camera: public DirectXChild {
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

    void Init();

    void SetParams(CameraConfig config);
    CameraConfig GetParams();

    void BuildView();
    void BuildProj();

    void Translate(DirectX::XMFLOAT3 p);
    void TranslateAbs(DirectX::XMFLOAT3 p);
    void TranslateLookAt(DirectX::XMFLOAT3 p);
    void TranslateLookAtAbs(DirectX::XMFLOAT3 p);

    void Rotate(DirectX::XMFLOAT3 r);
    void RotateAbs(DirectX::XMFLOAT3 r);

    void SetWorldMatrix(DirectX::XMMATRIX w);
    const ConstantBuffer& BuildConstantBuffer(float wparam=1.f);
    const ConstantBuffer& BuildConstantBuffer(DirectX::XMVECTOR wparam);
    void BindBuffer(Shader::ShaderType type, UINT slot);

    DirectX::XMFLOAT3 GetPosition();
    DirectX::XMFLOAT3 GetRotation();

    void SetViewMatrix(DirectX::XMMATRIX view);
    DirectX::XMMATRIX GetViewMatrix();

    void SetProjMatrix(DirectX::XMMATRIX proj);
    DirectX::XMMATRIX GetProjMatrix();

    inline float GetAspect() const { return cfg.fAspect; }
};
