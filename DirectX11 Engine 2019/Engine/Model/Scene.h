#pragma once

#include "pc.h"

// Types
#include "Engine Includes/Types.h"

// ECS
#include "ECS/ECSComponent.hpp"
#include "ECS/ECSSystem.hpp"
#include "ECS/ECS.hpp"

// Engine stuff
#include "Engine/Utility/Utils.h"
#include "Engine/ScopedMapper.h"
#include "Other/FileSystem.h"
#include "Engine/States/PipelineState.h"
#include "Engine/Materials/Texture.h"
#include "Engine/Materials/Sampler.h"
#include "Engine/DirectX/ConstantBuffer.h"
#include "Engine/DirectX/VertexBuffer.h"
#include "Engine/DirectX/IndexBuffer.h"
#include "Engine/Extensions/Safe.h"
#include "HighLevel/DirectX/Utlities.h"
#include "Other/DrawCall.h"
#include "Engine/Window/Window.h"

// Components
#include "Components/Components.h"

extern Window   *gWindow;
extern _DirectX *gDirectX;
extern Mouse    *gMouse;
extern Keyboard *gKeyboard;
extern Gamepad  *gGamepad[NUM_GAMEPAD];

float fsignf(float x);

struct CameraData {
    CameraComponent    *cCam{};
    TransformComponent *cTransf{};

    inline void SetView(mfloat4x4 mView) {
        cCam->mView = mView;
    }
    
    inline void SetProj(mfloat4x4 mProj) {
        cCam->mProj = mProj;
    }

    void UpdateCB(ConstantBuffer* cb) {
        // Copy data to CB
        ScopeMapConstantBufferCopy<CameraBuff> map(cb, (void*)&cCam->mView);
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

        uint32_t flags = ieee_uint32(dt);


    }

};

class VelocityIntegrationSystem: public BaseECSSystem {
public:
    VelocityIntegrationSystem();

    void UpdateComponents(float dt, BaseECSComponent** comp) override;

};

class StaticMeshRenderSystem: public BaseECSSystem {
public:
    StaticMeshRenderSystem();

    void UpdateComponents(float dt, BaseECSComponent** comp) override;
};

class MovementControlIntegrationSystem: public BaseECSSystem {
public:
    MovementControlIntegrationSystem();

    void UpdateComponents(float dt, BaseECSComponent** comp) override;
};

typedef std::vector<EntityHandle> EntityHandleList;

//struct CameraInstance {
//    EntityHandle    mHandle;
//    CameraData*     mCameraData{};
//    ConstantBuffer* cbCameraData{};
//};

// Reserved slots:
//  0 - Main player camera
//  1 - Light camera
//  2 - Ortho screen space camera
//  3 - For further use (TBD)
//  4 - For further use (TBD)
//  5 - For further use (TBD)
// 
// The rest is free to use
// Max value is 32
#define SCENE_MAX_CAMERA_COUNT 8
class Scene: public PipelineState<Scene> {
private:
    StaticMeshRenderSystem *gStaticMeshRenderSystem;
    VelocityIntegrationSystem *gVelocityIntegrationSystem;
    AnimatedMeshRenderSystem *gAnimatedMeshRenderSystem;
    MovementControlIntegrationSystem *gMovementControlIntegrationSystem;

    friend StaticMeshRenderSystem;

    EntityHandleList mOpaque{};
    EntityHandleList mCubemaps{};
    EntityHandleList mTransparent{};

    //std::vector<EntityHandle> mParticleEmitters;
    //std::vector<EntityHandle> mParticleSystems;

    //std::array<CameraInstance*, SCENE_MAX_CAMERA_COUNT> mCameraInst{};
    std::array<EntityHandle, SCENE_MAX_CAMERA_COUNT> mCamera{};

    ECSSystemList mRenderOpaqueList{}, mRenderCubemapList{}, mRenderTransparentList{};
    ECSSystemList mUpdateList{}, mRenderList{};

    ECS mECS{};

    std::array<CameraData*, SCENE_MAX_CAMERA_COUNT> mCameraData{};
    std::array<ConstantBuffer*, SCENE_MAX_CAMERA_COUNT> cbCameraData{};
    ConstantBuffer* cbTransform = nullptr;
    ConstantBuffer* cbMaterial = nullptr;
    ConstantBuffer* cbAmbientLight = nullptr;

    EntityHandle mAmbientLight{};
    EntityHandle mWorldLight{};

    uint32_t mMainCamera{};
    uint32_t bUpdatedCameraLists{};
    uint32_t bIsTransparentPass{};

    uint32_t mMaterialLayerStates = 0xFFFFFFFF;

    EntityHandleList LoadModelExternalStatic(const char* fname) {
        MaterialComponent mat{};
        mat._UseVertexColor = false;
        mat._FlipNormals    = false;
        mat._IsTransparent  = false;   
        mat._ShadowCaster   = true;
        mat._ShadowReceiver = true;

        mat._Alpha               = 1.f;
        mat._AlbedoMul           = 1.f;
        mat._NormalMul           = 1.f;
        mat._MetallnessMul       = 1.f;
        mat._RoughnessMul        = 1.f;
        mat._AmbientOcclusionMul = 1.f;
        mat._EmissionMul         = 1.f;

        TransformComponent transform;
        transform.mWorld = DirectX::XMMatrixIdentity();
        transform.vPosition = {};
        transform.vRotation = {};
        transform.vScale = { 1.f, 1.f, 1.f };

        // Load model
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(fname, aiProcess_Triangulate | aiProcess_CalcTangentSpace
                                                      | aiProcess_GenSmoothNormals);

        if( !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode ) {
            std::cout << "[Scene::LoadModelExternal]: Can't load model! (" << fname << ")" << std::endl;
            return {  };
        }

        // Process scene
        std::vector<MeshStaticComponent> mMeshList;
        ProcessNodeStatic(scene->mRootNode, scene, &mMeshList);

        // Done
        if( mMeshList.size() == 1 ) {
            return { mECS.MakeEntity(transform, mMeshList[0], mat) };
        }

        // Return list of entites
        EntityHandleList list;
        list.reserve(mMeshList.size());
        for( auto e : mMeshList ) {
            list.push_back(mECS.MakeEntity(transform, e, mat));
        }

        // Done
        return list;
    }

    void ProcessNodeStatic(aiNode* node, const aiScene* scene, std::vector<MeshStaticComponent>* MeshList) {
        // Process meshes
        for( size_t i = 0; i < node->mNumMeshes; i++ ) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            MeshList->push_back(ProcessMeshStatic(mesh, scene));
        }

        // Process children
        for( size_t i = 0; i < node->mNumChildren; i++ ) {
            ProcessNodeStatic(node->mChildren[i], scene, MeshList);
        }
    }

    MeshStaticComponent ProcessMeshStatic(aiMesh* inMesh, const aiScene* scene) {
        MeshStaticComponent mesh;

        // 
        std::vector<float3> Position;
        std::vector<float2> Texcoord;
        std::vector<float3> Normal;

        std::vector<uint32_t> Index;
        
        // Process vertices
        Position.reserve(inMesh->mNumVertices);
        Texcoord.reserve(inMesh->mNumVertices);
        Normal.reserve(  inMesh->mNumVertices);

        for( size_t i = 0; i < inMesh->mNumVertices; i++ ) {
            Position.push_back({ inMesh->mVertices[i].x, inMesh->mVertices[i].y, inMesh->mVertices[i].z });
            Normal  .push_back({ inMesh->mNormals [i].x, inMesh->mNormals [i].y, inMesh->mNormals [i].z });

            if( inMesh->mTextureCoords[0] ) {
                Texcoord.push_back({ inMesh->mTextureCoords[0][i].x, inMesh->mTextureCoords[0][i].y });
            } else {
                Texcoord.push_back({ 0, 0 });
            }
        }

        // Process indices
        uint32_t IndexNum = 0;
        Index.reserve(inMesh->mNumFaces * 3); // Assume that each face has at least 3 indices
        for( size_t i = 0; i < inMesh->mNumFaces; i++ ) {
            aiFace face = inMesh->mFaces[i];
            IndexNum += face.mNumIndices;

            for( size_t j = 0; j < face.mNumIndices; j++ ) {
                Index.push_back(face.mIndices[j]);
            }
        }

        // Create buffers
        mesh.mIndexBuffer = new IndexBuffer();
        mesh.mVBPosition  = new VertexBuffer();
        mesh.mVBTexcoord  = new VertexBuffer();
        mesh.mVBNormal    = new VertexBuffer();
        mesh.mReferenced  = false;

        mesh.mVBPosition->CreateDefault(Position.size(), sizeof(float3), &Position[0]);
        mesh.mVBTexcoord->CreateDefault(Texcoord.size(), sizeof(float2), &Texcoord[0]);
        mesh.mVBNormal->CreateDefault(  Normal  .size(), sizeof(float3), &Normal  [0]);

        mesh.mIndexBuffer->CreateDefault(IndexNum, &Index[0]);
        
        // Done
        return mesh;
    }

public:
    enum MaterialLayers {
        Default = 1,
        Clouds = 2,

        Aux0 = 4,
        Aux1 = 8,
        Aux2 = 16,
        Aux3 = 32,
        Aux4 = 64,
        Aux5 = 128,
        Aux6 = 256,
        Aux7 = 512,
        Aux8 = 1024,
        Aux9 = 2048,
        Aux10 = 4096,
        Aux11 = 8192,
        Aux12 = 16384,
        Aux14 = 32768,
        Aux15 = 1 << 16,
        Aux16 = 1 << 17,
        Aux17 = 1 << 19,
        Aux18 = 1 << 19,
        Aux19 = 1 << 20,
        Aux20 = 1 << 21,
        Aux21 = 1 << 22,
        Aux22 = 1 << 23,
        Aux23 = 1 << 24,
        Aux24 = 1 << 25,
        Aux25 = 1 << 26,
        Aux26 = 1 << 27,
        Aux27 = 1 << 28,
        Aux28 = 1 << 29,
        Aux29 = 1 << 30,
        Aux30 = 1 << 31,

        Count = 32,
        All = 0xFFFFFFFF
    };

    Scene() {
        // Create default lights
        AmbientLightComponent ambient;
        ambient._AmbientLightStrengh = 1.f;
        ambient._AmbientLightColor = float3(.1f, .1f, .1f);
        mAmbientLight = mECS.MakeEntity(ambient);

        WorldLightComponent world;
        world._WorldLightDirection = float3(1.f, 1.f, 1.f);
        world._WorldLightPosition = {};
        world._WorldLightColor = float3(.7f, .6f, .3f);
        mWorldLight = mECS.MakeEntity(world);

        // 
        char buff[128]{};
        bUpdatedCameraLists = false;

        for( uint32_t i = 0; i < SCENE_MAX_CAMERA_COUNT; i++ ) {
            sprintf_s(&buff[0], 128, "[Scene::Camera]: Constant Buffer #%u", i);

            cbCameraData[i] = new ConstantBuffer();
            cbCameraData[i]->CreateDefault(sizeof(CameraBuff));
            cbCameraData[i]->SetName(&buff[0]);

            mCameraData[i] = new CameraData();
        }

        cbAmbientLight = new ConstantBuffer();
        cbAmbientLight->CreateDefault(sizeof(AmbientLightBuff));
        cbAmbientLight->SetName("[Scene::AmbientLight]: ConstantBuffer");

        cbMaterial = new ConstantBuffer();
        cbMaterial->CreateDefault(sizeof(MaterialBuff));
        cbMaterial->SetName("[Scene::Material]: ConstantBuffer");

        cbTransform = new ConstantBuffer();
        cbTransform->CreateDefault(sizeof(TransformBuff));
        cbTransform->SetName("[Scene::MeshTransform]: ConstantBuffer");

        gVelocityIntegrationSystem        = new VelocityIntegrationSystem;
        gAnimatedMeshRenderSystem         = new AnimatedMeshRenderSystem;
        gStaticMeshRenderSystem           = new StaticMeshRenderSystem;
        gMovementControlIntegrationSystem = new MovementControlIntegrationSystem;

        // 
        ResetLists();
    }

    ~Scene() {
        for( auto cb : cbCameraData )
            if( cb ) {
                cb->Release();
                delete cb;
            }

        for( auto cd : mCameraData )
            if( cd ) delete cd;

        //SAFE_RELEASE_N(cbCameraData, SCENE_MAX_CAMERA_COUNT);
        SAFE_RELEASE(cbTransform);
        SAFE_RELEASE(cbMaterial);
        SAFE_RELEASE(cbAmbientLight);

        //SAFE_DELETE_N(mCameraData, SCENE_MAX_CAMERA_COUNT);
        SAFE_DELETE(gVelocityIntegrationSystem);
        SAFE_DELETE(gAnimatedMeshRenderSystem );
        SAFE_DELETE(gStaticMeshRenderSystem   );
        SAFE_DELETE(gMovementControlIntegrationSystem);

        // ECS
        for( uint32_t i = 0; i < SCENE_MAX_CAMERA_COUNT; i++ ) 
            if( mCamera[i] != NULL_HANDLE ) mECS.RemoveEntity(mCamera[i]);

        auto ReleaseMesh = [&](EntityHandle e, std::string_view name) {
            if( e == NULL_HANDLE ) return;
            auto static_mesh = GetComponent<MeshStaticComponent>(e);
            auto anim_mesh   = GetComponent<MeshAnimatedComponent>(e);
            //auto // TODO: Release material

            // Unload mesh
            if( static_mesh ) {
                if( !static_mesh->mReferenced ) {
                    SAFE_RELEASE(static_mesh->mIndexBuffer);

                    SAFE_RELEASE(static_mesh->mVBPosition);
                    SAFE_RELEASE(static_mesh->mVBTexcoord);
                    SAFE_RELEASE(static_mesh->mVBNormal);
                }
            } else if( anim_mesh ) {
                if( !static_mesh->mReferenced ) {
                    SAFE_RELEASE(anim_mesh->mIndexBuffer);

                    SAFE_RELEASE(anim_mesh->mVBPosition);
                    SAFE_RELEASE(anim_mesh->mVBTexcoord);
                    SAFE_RELEASE(anim_mesh->mVBNormal);

                    SAFE_RELEASE(anim_mesh->mVBWeights);
                    SAFE_RELEASE(anim_mesh->mVBJoints);
                }
            } else {
                printf_s("[Scene::~Scene]: Error occured during unloading %s mesh %u\n", name.data(), e);
            }

            mECS.RemoveEntity(e);
        };

        for( auto e : mOpaque ) ReleaseMesh(e, "opaque");
        for( auto e : mCubemaps ) ReleaseMesh(e, "cubemap");
        for( auto e : mTransparent ) ReleaseMesh(e, "transparent");

        mOpaque.clear();
        mCubemaps.clear();
        mTransparent.clear();

        mRenderOpaqueList.Clear();
        mRenderCubemapList.Clear();
        mRenderTransparentList.Clear();

        mUpdateList.Clear();
    }

    uint32_t GetEnabledMaterialLayers() const { return mMaterialLayerStates; }
    bool IsTransparentPass() const { return bIsTransparentPass; };

    // Set current scene as active
    // Everything will refere to active scene
    // Renderer, Update events, etc...
    inline void SetAsActive() { gState = this; }

    // Get component from local ECS
    template<typename T>
    T* GetComponent(EntityHandle handle) { return mECS.GetComponent<T>(handle); }

    template<typename T>
    void AddComponent(EntityHandle handle, T *comp) { mECS.AddComponent(handle, comp); }

    EntityHandleList Instantiate(EntityHandleList list) {
        EntityHandleList new_list;
        new_list.reserve(list.size());

        for( EntityHandle e : list ) {
            // Copy components & instantiate new Entity
            MeshStaticComponent* static_mesh_ref = GetComponent<MeshStaticComponent>(e);
            MeshAnimatedComponent* anim_mesh_ref = GetComponent<MeshAnimatedComponent>(e);
            TransformComponent transf_ref        = *GetComponent<TransformComponent>(e);
            MaterialComponent mat_ref            = *GetComponent<MaterialComponent>(e);

            MeshStaticComponent static_mesh{};
            MeshAnimatedComponent anim_mesh{};
            TransformComponent transf{};
            MaterialComponent mat{};

            // Copy transform data
            memcpy((void*)&transf.mWorld, (void*)&transf_ref.mWorld, sizeof(TransformBuff));

            // Copy material data
            memcpy((void*)&mat._IsTransparent, (void*)&mat_ref._IsTransparent, sizeof(MaterialBuff));
            memcpy((void*)&mat._AlbedoTex, (void*)&mat_ref._AlbedoTex, sizeof(uint64_t) * 6 * 2);

            EntityHandle ent = NULL_HANDLE;
            if( static_mesh_ref != NULL_HANDLE ) {
                static_mesh.mIndexBuffer = static_mesh_ref->mIndexBuffer;
                static_mesh.mVBPosition  = static_mesh_ref->mVBPosition;
                static_mesh.mVBTexcoord  = static_mesh_ref->mVBTexcoord;
                static_mesh.mVBNormal    = static_mesh_ref->mVBNormal;
                static_mesh.mReferenced  = true;

                ent = mECS.MakeEntity(transf, static_mesh, mat);
            } else if( anim_mesh_ref != NULL_HANDLE ) {
                anim_mesh.mIndexBuffer = anim_mesh_ref->mIndexBuffer;
                anim_mesh.mVBPosition  = anim_mesh_ref->mVBPosition;
                anim_mesh.mVBTexcoord  = anim_mesh_ref->mVBTexcoord;
                anim_mesh.mVBNormal    = anim_mesh_ref->mVBNormal;
                anim_mesh.mVBWeights   = anim_mesh_ref->mVBWeights;
                anim_mesh.mVBJoints    = anim_mesh_ref->mVBJoints;
                anim_mesh.mReferenced  = true;

                ent = mECS.MakeEntity(transf, anim_mesh, mat);
            }

            if( ent != NULL_HANDLE ) {
                new_list.push_back(ent);
                if( mat_ref._IsTransparent ) mTransparent.push_back(ent);
                else                     mOpaque.push_back(ent);
            }
        }

        return new_list;
    }

    void AmbientLight(float3 color, float strengh=1.f) {
        AmbientLightComponent* comp = GetComponent<AmbientLightComponent>(mAmbientLight);
        comp->_AmbientLightColor = color;
        comp->_AmbientLightStrengh = strengh;
    }

    void BindAmbientLight(Shader::ShaderType type, UINT slot=1) {
        AmbientLightComponent* comp = GetComponent<AmbientLightComponent>(mAmbientLight);
        {
            ScopeMapConstantBufferCopy<AmbientLightBuff> q(cbAmbientLight, &comp->_AmbientLightColor);
        }
        
        cbAmbientLight->Bind(type, slot);
    }

    // TODO: 
    // StructuredBuffer<float4x4> _WorldMatrices[MAX_INSTANCE_NUM];
    // DrawIndexedInstanced
    uint32_t AddOpaqueStaticInstance(EntityHandle handle, mfloat4x4 mWorld) {
        return 0; // Instance id
    }

    void AddOpaque(EntityHandleList list) {
        mOpaque.resize(list.size() + list.size());

        for( auto e : list ) 
            mOpaque.push_back(e);
    }
    
    void AddCubemaps(EntityHandleList list) {
        mCubemaps.resize(mCubemaps.size() + list.size());

        for( auto e : list )
            mCubemaps.push_back(e);
    }

    void AddTransparent(EntityHandleList list) {
        mTransparent.resize(mTransparent.size() + list.size());

        for( auto e : list ) {
            GetComponent<MaterialComponent>(e)->_IsTransparent = true;
            mTransparent.push_back(e);
        }
    }

    EntityHandleList LoadModelStatic(const char* fname) {
        std::string_view ext = path_ext(fname);

        if( ext == "obj" || ext == "dae" || ext == "fbx" ) {
            return LoadModelExternalStatic(fname);
        }

        // Other file types
        printf_s("[Scene::LoadModel]: Unsupported model format %s", ext.data());
        return {};
    }

    EntityHandleList LoadModelStaticOpaque(const char* fname, void(*Operator)(TransformComponent*)=[](TransformComponent*)->void{}) {
        EntityHandleList list = LoadModelStatic(fname);
        for( auto e : list ) Operator(GetComponent<TransformComponent>(e));

        AddOpaque(list);
        return list;
    }
    
    EntityHandleList LoadModelStaticTransparent(const char* fname) {
        EntityHandleList list = LoadModelStatic(fname);
        AddTransparent(list);
        return list;

    }

    inline ECS* GetECS() { return &mECS; };

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
        /*mRenderOpaqueList.AddSystem(*gAnimatedMeshRenderSystem);
        mRenderOpaqueList.AddSystem(*gStaticMeshRenderSystem);

        mRenderTransparentList.AddSystem(*gAnimatedMeshRenderSystem);
        mRenderTransparentList.AddSystem(*gStaticMeshRenderSystem);*/

        mRenderList.AddSystem(*gAnimatedMeshRenderSystem);
        mRenderList.AddSystem(*gStaticMeshRenderSystem);

        mUpdateList.AddSystem(*gMovementControlIntegrationSystem);
        mUpdateList.AddSystem(*gVelocityIntegrationSystem);
    }

    // Use GetECS() to get ECS context & then to create camera you self
    void AddCamera(uint32_t CameraIndex, EntityHandle entity) {
        if( mCamera[CameraIndex] != NULL_HANDLE ) mECS.RemoveEntity(mCamera[CameraIndex]);
        mCamera[CameraIndex] = entity;
    }

    void MakeCameraOrtho(uint32_t CameraIndex, float _near, float _far, float width, float height) {
        if( mCamera[CameraIndex] != NULL_HANDLE ) mECS.RemoveEntity(mCamera[CameraIndex]);

        TransformComponent transf;
        transf.vPosition = {};
        transf.vRotation = {};
        transf.mWorld = DirectX::XMMatrixIdentity();

        CameraComponent cam{};
        cam.bOrtho  = true;
        cam.fNear   = _near;
        cam.fFar    = _far;
        cam.fWidth  = width;
        cam.fHeight = height;

        mCamera[CameraIndex] = mECS.MakeEntity(transf, cam);
        UpdateCameraData(CameraIndex);
        mCameraData[CameraIndex]->Build();
        bUpdatedCameraLists = false;
    }

    void MakeCameraFOVH(uint32_t CameraIndex, float _near, float _far, float width, float height, float fovx) {
        if( mCamera[CameraIndex] != NULL_HANDLE ) mECS.RemoveEntity(mCamera[CameraIndex]);

        TransformComponent transf{};
        transf.vPosition = {};
        transf.vRotation = {};
        transf.mWorld = DirectX::XMMatrixIdentity();

        CameraComponent cam{};
        cam.bOrtho  = false;
        cam.fNear   = _near;
        cam.fFar    = _far;
        cam.fAspect = width / height;
        cam.fFOV_X  = fovx;
        cam.fFOV_Y  = 0.f;
        cam.fWidth  = width;
        cam.fHeight = height;

        mCamera[CameraIndex] = mECS.MakeEntity(transf, cam);
        bUpdatedCameraLists = false;
    }

    void UpdateMadeCameras() {
        for( uint32_t i = 0; i < SCENE_MAX_CAMERA_COUNT; i++ ) {
            if( mCamera[i] != NULL_HANDLE ) {
                UpdateCameraData(i);
                mCameraData[i]->Build();
            }
        }

        bUpdatedCameraLists = true;
    }
    
    void DefineCameraOrtho(uint32_t i, float _near, float _far, float width, float height) {
        if( mCamera[i] == NULL_HANDLE ) {
            MakeCameraOrtho(i, _near, _far, width, height);
        } else {
            mCameraData[i]->cCam->fWidth  = width;
            mCameraData[i]->cCam->fHeight = height;
            mCameraData[i]->cCam->fAspect = width / height;
            mCameraData[i]->cCam->fNear   = _near;
            mCameraData[i]->cCam->fFar    = _far;
            mCameraData[i]->BuildProj();
        }
    }

    void DefineCameraFOVH(uint32_t i, float _near, float _far, float width, float height, float fovx) {
        if( mCamera[i] == NULL_HANDLE ) {
            MakeCameraFOVH(i, _near, _far, width, height, fovx);
        } else {
            mCameraData[i]->cCam->fWidth  = width;
            mCameraData[i]->cCam->fHeight = height;
            mCameraData[i]->cCam->fAspect = width / height;
            mCameraData[i]->cCam->fNear   = _near;
            mCameraData[i]->cCam->fFar    = _far;
            mCameraData[i]->BuildProj();
        }
    }

    inline EntityHandle GetActiveCameraHandle() const { return mCamera[mMainCamera]; }
    inline uint32_t GetActiveCamera() const { return mMainCamera; }
    inline void SetActiveCamera(uint32_t i) { mMainCamera = i; }

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
    
    // Try to switch to std::array
    void UpdateCameraData(uint32_t CameraIndex) {
        if( mCamera[CameraIndex] == NULL_HANDLE ) return;
        mCameraData[CameraIndex]->cCam    = GetComponent<CameraComponent>   (mCamera[CameraIndex]);
        mCameraData[CameraIndex]->cTransf = GetComponent<TransformComponent>(mCamera[CameraIndex]);
    }
    
    void BindCamera(uint32_t CameraIndex, uint32_t types=Shader::Vertex, uint32_t slot=1) {
        // To be sure
        if( !bUpdatedCameraLists ) UpdateMadeCameras();

        // Copy component refs to mCameraData from Camera entity
        UpdateCameraData(CameraIndex);

        // Copy data to CB
        mCameraData[CameraIndex]->UpdateCB(cbCameraData[CameraIndex]);

        // Bind CB
        cbCameraData[CameraIndex]->Bind(types, slot);
    }

    inline CameraData* GetCamera(uint32_t CameraIndex) const { return mCameraData[CameraIndex]; }

    void SetLayersState(uint32_t Layers, bool Enable) {
        for( uint i = 0, j = 1; i < Layers; j = 1 <<++ i ) {
            if( Layers & j ) {
                if( Enable ) {
                    mMaterialLayerStates |= j;
                } else {
                    mMaterialLayerStates &= ~j;
                }
            }
        }
    }

    inline void SetLayersState(uint32_t NewState) { mMaterialLayerStates = NewState; }

    void Render(uint32_t flags=0, Shader::ShaderType type_transf=Shader::Vertex, Shader::ShaderType type_tex=Shader::Pixel) {
        flags |= type_transf << (32 - Shader::Count);
        flags |= type_tex << (32 - Shader::Count * 2);

        bIsTransparentPass = (flags & RendererFlags::OpacityPass);
        mECS.UpdateSystems(mRenderList, ieee_float(flags));
    }

    void RenderCubemap(uint32_t flags=0, Shader::ShaderType type_transf=Shader::Vertex, Shader::ShaderType type_tex=Shader::Pixel) {
        flags |= type_transf << (32 - Shader::Count);
        flags |= type_tex    << (32 - Shader::Count * 2);

        bIsTransparentPass = false;
        mECS.UpdateSystems(mRenderCubemapList, ieee_float(flags));
    }

    void RenderOpaque(uint32_t flags=0, Shader::ShaderType type_transf=Shader::Vertex, Shader::ShaderType type_tex=Shader::Pixel) {
        flags |= type_transf << (32 - Shader::Count);
        flags |= type_tex    << (32 - Shader::Count * 2);

        bIsTransparentPass = false;
        mECS.UpdateSystems(mRenderOpaqueList, ieee_float(flags));
    }

    void RenderTransparent(uint32_t flags=0, Shader::ShaderType type_transf=Shader::Vertex, Shader::ShaderType type_tex=Shader::Pixel) {
        flags |= type_transf << (32 - Shader::Count);
        flags |= type_tex    << (32 - Shader::Count * 2);

        bIsTransparentPass = true;
        mECS.UpdateSystems(mRenderTransparentList, ieee_float(flags));
    }

    void Update(float dt) {
        mECS.UpdateSystems(mUpdateList, dt);
    }
};
