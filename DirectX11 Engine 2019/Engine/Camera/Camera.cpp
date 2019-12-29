#include "pc.h"
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
    if( !cb || 
#ifdef _WIN64
        (uint64_t)cb == 0xCCCCCCCCCCCCCCCC 
#else
       // TODO: Check
        (uint64_t)cb == 0xCCCCCCCC
#endif
       ) {
        cb = new ConstantBuffer();
        cb->CreateDefault(sizeof(BufferMatrix));
        cb->SetName("[CB]: Camera Matrix");
    }
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
    if( cfg.Ortho ) {
        mProj = DirectX::XMMatrixOrthographicOffCenterLH(0.f, cfg.ViewW, cfg.ViewH, 0.f, cfg.fNear, cfg.fFar);
    } else {
        mProj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(cfg.FOV / cfg.fAspect), cfg.fAspect, cfg.fFar, cfg.fNear);
    }
}

void Camera::Translate(DirectX::XMFLOAT3 p) {
    pPos.x += p.x;
    pPos.y += p.y;
    pPos.z += p.z;
}

void Camera::TranslateAbs(DirectX::XMFLOAT3 p) {
    pPos.x = p.x;
    pPos.y = p.y;
    pPos.z = p.z;
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

void Camera::TranslateLookAtAbs(DirectX::XMFLOAT3 p) {
    DirectX::XMFLOAT3 dp = {p.x - pPos.x, p.y - pPos.y, p.z - pPos.z};
    TranslateLookAt(dp);
}

void Camera::Rotate(DirectX::XMFLOAT3 r) {
    pRot.x += r.x;
    pRot.y += r.y;
    pRot.z += r.z;
}

void Camera::RotateAbs(DirectX::XMFLOAT3 r) {
    pRot.x = r.x;
    pRot.y = r.y;
    pRot.z = r.z;
}

const ConstantBuffer& Camera::BuildConstantBuffer(float wparam) {
    // Map buffer
    BufferMatrix* ptr = (BufferMatrix*)cb->Map();
    
    ptr->mWorld = mWorld;
    ptr->mView  = mView;
    ptr->mProj  = mProj;
    ptr->vPosition = { pPos.x, pPos.y, pPos.z, wparam };

    // Unmap
    cb->Unmap();
    return *cb;
}

const ConstantBuffer& Camera::BuildConstantBuffer(DirectX::XMVECTOR wparam) {
    // Map buffer
    BufferMatrix* ptr = (BufferMatrix*)cb->Map();
    
    ptr->mWorld = mWorld;
    ptr->mView  = mView;
    ptr->mProj  = mProj;
    ptr->vPosition = wparam;

    // Unmap
    cb->Unmap();
    return *cb;
}

void Camera::BindBuffer(UINT type, UINT slot) {
    cb->Bind(type, slot);
}
