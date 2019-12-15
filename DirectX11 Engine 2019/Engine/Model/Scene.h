#pragma once

#include <vector>

// ECS
#include "ECS/ECSComponent.hpp"
#include "ECS/ECSSystem.hpp"
#include "ECS/ECS.hpp"

// Types
#include <DirectXMath.h>
#include "Engine Includes/Types.h"
#include "Other/FloatTypeMath.h"

// Components
#include "Components/Components.h"

// 
#include "Engine/RendererFlags.h"
#include "Engine/ScopedMapper.h"

struct CameraData {
    CameraComponent    *cCam;
    TransformComponent *cTransf;

    inline void SetView(mfloat4x4 mView) {
        cCam->mView = mView;
    }
    
    inline void SetProj(mfloat4x4 mProj) {
        cCam->mProj = mProj;
    }

    void UpdateCB(ConstantBuffer* cb) {
        // Copy data to CB
        ScopeMapConstantBufferCopy<CameraBuff> map(cb, &cCam->mView);
    }

    void BuildView() {
        using namespace DirectX;

        // 
        mfloat3 EyePos = { cTransf->vPosition.x, cTransf->vPosition.y, cTransf->vPosition.z },
                Focus  = { 0.f, 0.f, 1.f },
                Up     = { 0.f, 1.f, 0.f };

        // Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
        float pitch = cTransf->vRotation.x * 0.0174532925f;
        float yaw   = cTransf->vRotation.y * 0.0174532925f;
        float roll  = cTransf->vRotation.z * 0.0174532925f;

        // Create the rotation matrix from the yaw, pitch, and roll values.
        XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

        // Transform the lookAt and up vector by the rotation matrix so the view is correctly rotated at the origin.
        mfloat3 lookAt = XMVector3TransformCoord(Focus, rotationMatrix);
        mfloat3 up = XMVector3TransformCoord(Up, rotationMatrix);

        // Translate the rotated camera position to the location of the viewer.
        lookAt = EyePos + lookAt;

        // Build view matrix
        cCam->mView = XMMatrixLookAtLH(EyePos, lookAt, Up);
    }

    void BuildProj() {
        using namespace DirectX;

        if( cCam->bOrtho ) {
            cCam->mProj = XMMatrixOrthographicOffCenterLH(0.f, cCam->fWidth, cCam->fHeight, 0.f, cCam->fNear, cCam->fFar);
        } else {
            cCam->mProj = XMMatrixPerspectiveFovLH(XMConvertToRadians(cCam->fFOV_X / cCam->fAspect), 
                                                   cCam->fAspect, cCam->fFar, cCam->fNear);
        }
    }

    inline void Build() {
        BuildView();
        BuildProj();
    }

    inline void ViewIdentity() { SetView(DirectX::XMMatrixIdentity()); }
    inline void ProjIdentity() { SetProj(DirectX::XMMatrixIdentity()); }
};

class AnimatedMeshRenderSystem: public BaseECSSystem {
public:
    AnimatedMeshRenderSystem(): BaseECSSystem() {
        AddComponentType(TransformComponent::_ID); // 0
        AddComponentType(MeshAnimatedComponent::_ID); // 1
    }

    void UpdateComponents(float dt, BaseECSComponent** comp) override {
        TransformComponent* transform = (TransformComponent*)comp[0];
        MeshAnimatedComponent* mesh   = (MeshAnimatedComponent*)comp[1];


    }

} *gAnimatedMeshRenderSystem;

class StaticMeshRenderSystem: public BaseECSSystem {
public:
    StaticMeshRenderSystem(): BaseECSSystem() {
        AddComponentType(TransformComponent::_ID);  // 0
        AddComponentType(MeshStaticComponent::_ID); // 1
    }

    void UpdateComponents(float dt, BaseECSComponent** comp) override {
        TransformComponent* transform = (TransformComponent*)comp[0];
        MeshStaticComponent* mesh     = (MeshStaticComponent*)comp[1];


    }

} *gStaticMeshRenderSystem;

class VelocityIntegrationSystem: public BaseECSSystem {
public:
    VelocityIntegrationSystem() {
        AddComponentType(TransformComponent::_ID); // 0
        AddComponentType(VelocityComponent::_ID);  // 1
    }

    void UpdateComponents(float dt, BaseECSComponent** comp) override {
        TransformComponent* transform = (TransformComponent*)comp[0];
        VelocityComponent* velocity   = (VelocityComponent*)comp[1];

        // TODO: Fix +=
        transform->vPosition += velocity->vDirection * velocity->fVelocity * dt;
        velocity->fVelocity  += velocity->fAcceleration * dt;

        velocity->fAcceleration *= .9f;
        velocity->fVelocity     *= .9f;
    }

} *gVelocityIntegrationSystem;

class Scene {
private:
    std::vector<EntityHandle> mOpaque;
    std::vector<EntityHandle> mCubemaps;
    std::vector<EntityHandle> mTransparent;

    //std::vector<EntityHandle> mParticleEmitters;
    //std::vector<EntityHandle> mParticleSystems;

    EntityHandle mCamera;

    ECSSystemList mRenderOpaqueList, mRenderCubemapList, mRenderTransparentList;
    ECSSystemList mUpdateList;

    ECS mECS;

    // 
    template<typename T>
    T* GetComponent(EntityHandle handle) { return mECS.GetComponent<T>(handle); }

    CameraData* mCameraData = nullptr;
    ConstantBuffer* cbCameraData;

public:
    Scene() {
        cbCameraData = new ConstantBuffer();
        cbCameraData->CreateDefault(sizeof(CameraData));
        cbCameraData->SetName("[Scene::Camera]: Constant Buffer");

        mCameraData = new CameraData();

        gVelocityIntegrationSystem = new VelocityIntegrationSystem;
        gAnimatedMeshRenderSystem  = new AnimatedMeshRenderSystem;
        gStaticMeshRenderSystem    = new StaticMeshRenderSystem;
    }

    ~Scene() {
        SAFE_RELEASE(cbCameraData);
        SAFE_DELETE(mCameraData);

        // ECS
        mECS.RemoveEntity(mCamera);
        for( auto e : mOpaque      ) mECS.RemoveEntity(e);
        for( auto e : mCubemaps    ) mECS.RemoveEntity(e);
        for( auto e : mTransparent ) mECS.RemoveEntity(e);

        mOpaque.clear();
        mCubemaps.clear();
        mTransparent.clear();

        mRenderOpaqueList.Clear();
        mRenderCubemapList.Clear();
        mRenderTransparentList.Clear();

        mUpdateList.Clear();
    }

    void AddOpaque(std::initializer_list<EntityHandle> list) {
        mOpaque.resize(list.size() + list.size());

        for( auto e : list )
            mOpaque.push_back(e);
    }
    
    void AddCubemaps(std::initializer_list<EntityHandle> list) {
        mCubemaps.resize(mCubemaps.size() + list.size());

        for( auto e : list )
            mCubemaps.push_back(e);
    }

    void AddTransparent(std::initializer_list<EntityHandle> list) {
        mTransparent.resize(mTransparent.size() + list.size());

        for( auto e : list )
            mTransparent.push_back(e);
    }

    void ClearRenderLists() {
        mRenderOpaqueList.Clear();
        mRenderCubemapList.Clear();
        mRenderTransparentList.Clear();

        mUpdateList.Clear();
    }

    void ClearUpdateList() {
        mUpdateList.Clear();
    }

    // Run only once to reset all system lists
    void ResetLists() {
        ClearRenderLists();
        ClearUpdateList();

        //mRenderCubemapList.;
        mRenderOpaqueList.AddSystem(*gAnimatedMeshRenderSystem);
        mRenderTransparentList.AddSystem(*gStaticMeshRenderSystem);

        mUpdateList.AddSystem(*gVelocityIntegrationSystem);
    }

    void AddCamera(EntityHandle entity) {
        mCamera = entity;
    }

    void MakeCameraOrtho(float _near, float _far, float width, float height) {
        if( mCamera != NULL_HANDLE ) mECS.RemoveEntity(mCamera);

        TransformComponent transf;
        transf.vPosition = {};
        transf.vRotation = {};

        CameraComponent cam;
        cam.bOrtho  = true;
        cam.fNear   = _near;
        cam.fFar    = _far;
        cam.fWidth  = width;
        cam.fHeight = height;

        mCamera = mECS.MakeEntity(transf, cam);
        UpdateCameraData();
    }

    void MakeCameraFOVH(float _near, float _far, float width, float height, float fovx) {
        if( mCamera != NULL_HANDLE ) mECS.RemoveEntity(mCamera);

        TransformComponent transf;
        transf.vPosition = {};
        transf.vRotation = {};

        CameraComponent cam;
        cam.bOrtho  = false;
        cam.fNear   = _near;
        cam.fFar    = _far;
        cam.fAspect = width / height;
        cam.fFOV_X  = fovx;
        cam.fFOV_Y  = 0.f;

        mCamera = mECS.MakeEntity(transf, cam);
        UpdateCameraData();
    }

    /*void AddCameras(std::initializer_list<EntityHandle> list) {
        mCameras.resize(mCameras.size() + list.size());


        for( auto e : list )
            mCameras.push_back(e);
    }*/

    /*void SetActiveCamera(uint32_t index) {

    }*/

    //void RemoveOpaque();
    //void RemoveCubemap();
    //void RemoveTransparent();
    
    void UpdateCameraData() {
        mCameraData->cCam    = GetComponent<CameraComponent>   (mCamera);
        mCameraData->cTransf = GetComponent<TransformComponent>(mCamera);
    }
    
    void BindCamera(uint32_t types=Shader::Vertex, uint32_t slot=0) {
        // Copy component refs to mCameraData from Camera entity
        UpdateCameraData();

        // Copy data to CB
        mCameraData->UpdateCB(cbCameraData);

        // Bind CB
        cbCameraData->Bind(types, slot);
    }

    inline CameraData* GetCamera() const { return mCameraData; }

    void RenderCubemap(uint32_t flags) {
        BindCamera();
        mECS.UpdateSystems(mRenderCubemapList, 0.f);
    }

    void RenderOpaque(uint32_t flags) {
        BindCamera();
        mECS.UpdateSystems(mRenderOpaqueList, 0.f);
    }

    void RenderTransparent(uint32_t flags) {
        BindCamera();
        mECS.UpdateSystems(mRenderTransparentList, 0.f);
    }

    void Update(float dt) {
        mECS.UpdateSystems(mUpdateList, dt);
    }
};
