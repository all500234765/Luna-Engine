#include "Camera.h"

Camera::Camera(): pPos(DirectX::XMFLOAT3({0, 0, 0})), pRot(DirectX::XMFLOAT3({0, 0, 0})) {
    Init();
}

Camera::Camera(DirectX::XMFLOAT3 p): pPos(p), pRot(DirectX::XMFLOAT3({0, 0, 0})) {
    Init();
}

Camera::Camera(DirectX::XMFLOAT3 p, DirectX::XMFLOAT3 r) : pPos(p), pRot(r) {
    Init();
}

void Camera::Init() {
    cb = new ConstantBuffer();
    cb->CreateDefault(sizeof(BufferMatrix));
    cb->SetName("Matrix Constant Buffer");
}

void Camera::SetParams(CameraConfig config) {
    cfg = config;
}

CameraConfig Camera::GetParams() {
    return cfg;
}

void Camera::BuildView() {
    using namespace DirectX;

    // 
    XMVECTOR EyePos = {pPos.x, pPos.y, pPos.z},
             Focus = {0., 0., 1.},
             Up = {0.f, 1.f, 0.f};

    // Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
    float pitch = pRot.x * 0.0174532925f;
    float yaw   = pRot.y * 0.0174532925f;
    float roll  = pRot.z * 0.0174532925f;

    // Create the rotation matrix from the yaw, pitch, and roll values.
    XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

    // Transform the lookAt and up vector by the rotation matrix so the view is correctly rotated at the origin.
    XMVECTOR lookAt = XMVector3TransformCoord(Focus, rotationMatrix);
    XMVECTOR up = XMVector3TransformCoord(Up, rotationMatrix);

    // Translate the rotated camera position to the location of the viewer.
    lookAt = EyePos + lookAt;

    // Build view matrix
    mView = XMMatrixLookAtLH(EyePos, lookAt, Up);
}

void Camera::BuildProj() {
    mProj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(cfg.FOV / cfg.fAspect), cfg.fAspect, cfg.fNear, cfg.fFar);
}

void Camera::Translate(DirectX::XMFLOAT3 p) {
    pPos.x += p.x;
    pPos.y += p.y;
    pPos.z += p.z;
}

void Camera::TranslateLookAt(DirectX::XMFLOAT3 p) {
    using namespace DirectX;

    // Get angles in radians
    float pr = XMConvertToRadians(pRot.x);
    float yr = XMConvertToRadians(pRot.y);
    float qr = yr - XMConvertToRadians(90.f * (p.z / fabsf(p.z)));

    // Move the direction we looking at
    XMFLOAT3 q = XMFLOAT3(
        // X
        (p.x * sinf(yr) + p.z * cosf(yr)) * cosf(pr),

        // Y
        -p.x * sinf(pr), 

        // Z
        (p.x * cosf(yr) - p.z * sinf(yr)) * cosf(pr)
    );

    pPos.x += q.x;
    pPos.y += q.y;
    pPos.z += q.z;
}

void Camera::Rotate(DirectX::XMFLOAT3 r) {
    pRot.x += r.x;
    pRot.y += r.y;
    pRot.z += r.z;
}

void Camera::SetWorldMatrix(const DirectX::XMMATRIX& w) {
    mWorld = w;
}

const ConstantBuffer& Camera::BuildConstantBuffer() {
    // Map buffer
    BufferMatrix* ptr = (BufferMatrix*)cb->Map();

    // Transpose the matrices to prepare them for the shader.
    ptr->mWorld = /*DirectX::XMMatrixTranspose*/(mWorld);
    ptr->mView  = /*DirectX::XMMatrixTranspose*/(mView);
    ptr->mProj  = /*DirectX::XMMatrixTranspose*/(mProj);

    // Unmap
    cb->Unmap();
    return *cb;
}

void Camera::BindBuffer(Shader::ShaderType type, UINT slot) {
    cb->Bind(type, slot);
}

DirectX::XMFLOAT3 Camera::GetPosition() {
    return pPos;
}

void Camera::SetViewMatrix(DirectX::XMMATRIX view) {
    mView = view;
}

DirectX::XMMATRIX Camera::GetViewMatrix() {
    return mView;
}

void Camera::SetProjMatrix(DirectX::XMMATRIX proj) {
    mProj = proj;
}

DirectX::XMMATRIX Camera::GetProjMatrix() {
    return mProj;
}
