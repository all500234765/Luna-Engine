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
#include "Engine/Scene/Texture.h"
#include "Engine/Scene/Sampler.h"
#include "Engine/DirectX/StructuredBuffer.h"
#include "Engine/DirectX/ConstantBuffer.h"
#include "Engine/DirectX/VertexBuffer.h"
#include "Engine/DirectX/IndexBuffer.h"
#include "Engine/Extensions/Safe.h"
#include "HighLevel/DirectX/Utlities.h"
#include "Other/DrawCall.h"
#include "Engine/Window/Window.h"
#include "Engine/States/TopologyState.h"

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
        cCam->mInvView = XMMatrixInverse(&XMMatrixDeterminant(cCam->mView), cCam->mView);
    }
    
    inline void SetProj(mfloat4x4 mProj) {
        cCam->mProj = mProj;
        cCam->mInvProj = XMMatrixInverse(&XMMatrixDeterminant(cCam->mProj), cCam->mProj);
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
        cCam->mInvView = XMMatrixInverse(&XMMatrixDeterminant(cCam->mView), cCam->mView);
    }

    void BuildProj() {
        using namespace DirectX;

        if( cCam->bOrtho ) {
            cCam->mProj = XMMatrixOrthographicOffCenterLH(0.f, cCam->fWidth, cCam->fHeight, 0.f, cCam->fNear, cCam->fFar);
        } else {
            cCam->mProj = XMMatrixPerspectiveFovLH(XMConvertToRadians(cCam->fFOV_X / cCam->fAspect), 
                                                   cCam->fAspect, cCam->fFar, cCam->fNear);
        }

        cCam->mInvProj = XMMatrixInverse(&XMMatrixDeterminant(cCam->mProj), cCam->mProj);
    }

    inline void Build() {
        BuildView();
        BuildProj();
    }

    inline void ViewIdentity() { SetView(DirectX::XMMatrixIdentity()); cCam->mInvView = cCam->mView; }
    inline void ProjIdentity() { SetProj(DirectX::XMMatrixIdentity()); cCam->mInvProj = cCam->mProj; }
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

// Place holders
class SpotLightShadowVolumeRenderSystem: public BaseECSSystem {
public:
    SpotLightShadowVolumeRenderSystem();

    void UpdateComponents(float dt, BaseECSComponent** comp) override;

};

class SpotLightFogVolumeRenderSystem: public BaseECSSystem {
public:
    SpotLightFogVolumeRenderSystem();

    void UpdateComponents(float dt, BaseECSComponent** comp) override;

};

class WorldLightVolumeRenderSystem: public BaseECSSystem {
public:
    WorldLightVolumeRenderSystem();

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
// Max recommended value is 32
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
    ConstantBuffer* cbTransform    = nullptr;
    ConstantBuffer* cbMaterial     = nullptr;
    ConstantBuffer* cbAmbientLight = nullptr;
    ConstantBuffer* cbWorldLight   = nullptr;

    EntityHandle mAmbientLight{};
    EntityHandle mWorldLight{};

    uint32_t mMainCamera{};
    uint32_t bUpdatedCameraLists{};
    uint32_t bIsTransparentPass{};

    Texture *mSkybox;

    uint32_t mMaterialLayerStates = 0xFFFFFFFF;

    // Lights
    EntityHandleList mPointLightStaticList, mPointLightDynamicList;
    StructuredBuffer<PointLightBuff> *sbPointLightStaticBuffer, *sbPointLightDynamicBuffer;
    uint mPointLightListStaticUpdate = true;

    // 
    bool mMeshSRV{};

    MaterialComponent DefaultMaterialComp() const {
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

        mat._RoughnessSampl        = nullptr;
        mat._EmissionSampl         = nullptr;
        mat._AmbientOcclusionSampl = nullptr;

        return mat;
    }

    TransformComponent DefaultTransformComp() const {
        TransformComponent transform{};
        transform.mWorld    = DirectX::XMMatrixIdentity();
        transform.vPosition = { 0.f, 0.f, 0.f };
        transform.vRotation = { 0.f, 0.f, 0.f };
        transform.vScale    = { 1.f, 1.f, 1.f };

        return transform;
    }

    typedef std::map<std::string, std::pair<aiTextureType, std::vector<uint32_t>>> LoaderTextureList;
    EntityHandleList LoadModelExternalStatic(const char* fname, ECS* ecs, uint32_t flags) {

        TransformComponent transform = DefaultTransformComp();

        // Load model
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(fname, aiProcess_Triangulate | aiProcess_CalcTangentSpace
                                                      | (0*aiProcess_GenSmoothNormals) | flags);

        if( !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode ) {
            std::cout << "[Scene::LoadModelExternal]: Can't load model! (" << fname << ")" << std::endl;
            return {  };
        }

        // Process scene
        std::vector<MeshStaticComponent> mMeshList;
        std::vector<MaterialComponent> mMatList;
        LoaderTextureList mTextureFileList;

        uint32_t index = 0u;
        ProcessNodeStatic(scene->mRootNode, scene, &mMeshList, &mMatList, &mTextureFileList, index);

        // Get current model's directory
        char drive[_MAX_DRIVE];
        char dir[_MAX_DIR];
        char fnm[_MAX_FNAME];
        char ext[_MAX_EXT];

        WCHAR cdir[_MAX_DIR];
        GetCurrentDirectory(_MAX_DIR, cdir);
        std::string ndir = narrow(cdir) + "/";

        _splitpath((ndir + fname).c_str(), drive, dir, fnm, ext);

        std::string path = "";
        path = drive;
        path += dir;

        _splitpath((ndir + "../Textures/").c_str(), drive, dir, fnm, ext);
        
        std::string tpath = "";
        tpath = drive;
        tpath += dir;

        // Load textures & attach to materials
        for( LoaderTextureList::iterator e = mTextureFileList.begin(); e != mTextureFileList.end(); e++ ) {
            LoaderTextureList::mapped_type data = e->second;
            std::string fnme = e->first;
            Texture *texture = 0;

            // Build paths
            std::string file1 = path + fnme;
            std::string file2 = tpath + fnme;

            // Search in current directory
            bool flag = false;
            if( file_exists(file1) ) {
                texture = new Texture(file1, tf_MipMaps*1);
                flag = true;
            } else if( file_exists(file2) ) {
                texture = new Texture(file2, tf_MipMaps*0);
                flag = true;
            }

            if( flag ) {
                // Assign texture
                switch( data.first ) {
#define MIND(type, tex, ts, v) case type: for( uint32_t mind : data.second ) { mMatList[mind].tex = texture; mMatList[mind].ts = v; } break;
                    case aiTextureType_NORMALS: 
                        MIND(aiTextureType_HEIGHT, _NormalTex, _Norm, 2);

                    MIND(aiTextureType_DIFFUSE  , _AlbedoTex          , _Alb  , 2);
                    MIND(aiTextureType_SHININESS, _RoughnessTex       , _Rough, 2);
                    MIND(aiTextureType_LIGHTMAP , _AmbientOcclusionTex, _AO   , 2);
                    MIND(aiTextureType_EMISSIVE , _EmissionTex        , _Emis , 2);
                    MIND(aiTextureType_UNKNOWN  , _MetallicTex        , _Metal, 2 | 4);

#undef MIND
                }

                printf_s("[Scene::ModelLoader]: Loaded texture. [%s]\n", fnme.c_str());
            } else {
                printf_s("[Scene::ModelLoader]: Failed to load texture! [%s]\n", fnme.c_str());
            }
        }

        // Done
        if( mMeshList.size() == 1 ) {
            return { ecs->MakeEntity(transform, mMeshList[0], mMatList[0]) };
        }

        // Return list of entites
        EntityHandleList list;
        list.reserve(mMeshList.size());
        for( uint32_t i = 0; i < mMeshList.size(); i++ ) {
            list.push_back(ecs->MakeEntity(transform, mMeshList[i], mMatList[i]));
        }

        // Done
        return list;
    }

    void ProcessNodeStatic(aiNode* node, const aiScene* scene, std::vector<MeshStaticComponent>* MeshList, 
                           std::vector<MaterialComponent>* MatList, 
                           LoaderTextureList* TextureList,
                           uint32_t& index) {
        // Process meshes
//#pragma omp parallel for num_threads(4)
        for( int32_t i = 0; i < (int32_t)node->mNumMeshes; i++ ) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            MeshList->push_back(ProcessMeshStatic(mesh, scene, MatList, TextureList, index));
            index++;
        }

        // Process children
        for( size_t i = 0; i < node->mNumChildren; i++ ) {
            ProcessNodeStatic(node->mChildren[i], scene, MeshList, MatList, TextureList, index);
        }
    }

    MeshStaticComponent ProcessMeshStatic(aiMesh* inMesh, const aiScene* scene, 
                           std::vector<MaterialComponent>* MatList, 
                           LoaderTextureList* TextureList, 
                           uint32_t index) {
        MaterialComponent mat = DefaultMaterialComp();
        MeshStaticComponent mesh{};

        // 
        std::vector<float3> Position;
        std::vector<float2> Texcoord;
        std::vector<float3> Normal;
        std::vector<float3> Tangent;

        std::vector<uint32_t> Index;
        
        // Process vertices
        Position.reserve(inMesh->mNumVertices);
        Texcoord.reserve(inMesh->mNumVertices);
        Tangent.reserve( inMesh->mNumVertices);
        Normal.reserve(  inMesh->mNumVertices);

        for( size_t i = 0; i < inMesh->mNumVertices; i++ ) {
            Position.push_back({ inMesh->mVertices[i].x, inMesh->mVertices[i].y, inMesh->mVertices[i].z });
            Normal  .push_back({ inMesh->mNormals [i].x, inMesh->mNormals [i].y, inMesh->mNormals [i].z });
            
            if( inMesh->mTangents ) {
                Tangent.push_back({ inMesh->mTangents[i].x, inMesh->mTangents[i].y, inMesh->mTangents[i].z });
            } else {
                Tangent.push_back({ 0, 0, 0 });
            }

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

        // Material
        uint32_t mat_index = inMesh->mMaterialIndex;
        aiMaterial* m = scene->mMaterials[mat_index];

        // Opacity
        ai_real alpha;
        m->Get(AI_MATKEY_OPACITY, alpha);
        if( alpha < 0.f ) alpha = 1.f;
        mat._Alpha = alpha;
        if( mat._Alpha < 1.f ) { mat._IsTransparent = 1.f; }

        // Gather material textures
        auto AddTexture = [&TextureList, index, m](aiTextureType type, uint32_t i=0u) {
            aiString fname;
            m->GetTexture(type, i, &fname);
            std::string s = fname.C_Str();
            //printf_s(" - %u:%u\n", type, m->GetTextureCount(type));

            // No texture found
            if( !strcmp(fname.C_Str(), "") ) return false;
            LoaderTextureList::iterator it = TextureList->find(s);

            if( it == TextureList->end() ) {
                //printf_s("%s; %u\n", s.c_str(), type);

                // Add
                TextureList->insert_or_assign(s, LoaderTextureList::mapped_type({ aiTextureType(type + i * (AI_TEXTURE_TYPE_MAX + 1)), { index } }));
                return true;
            }

            TextureList->at(s).second.push_back(index);
            return true;
        };

        //printf_s("-------------\n");

        AddTexture(aiTextureType_DIFFUSE  ); // Albedo
        AddTexture(aiTextureType_SHININESS); // Roughness
        AddTexture(aiTextureType_LIGHTMAP ); // Ambient Occlusion / Lightmap
        AddTexture(aiTextureType_EMISSIVE ); // Emission
        AddTexture(aiTextureType_UNKNOWN  ); // Metall Roughness

        // Normals
        if( !AddTexture(aiTextureType_HEIGHT) )
            AddTexture(aiTextureType_NORMALS);

        AddTexture(aiTextureType_OPACITY);

        /*AddTexture(aiTextureType_DISPLACEMENT);
        AddTexture(aiTextureType_AMBIENT);
        AddTexture(aiTextureType_SPECULAR);
        AddTexture(aiTextureType_REFLECTION);

        AddTexture(aiTextureType_BASE_COLOR);
        AddTexture(aiTextureType_NORMAL_CAMERA);
        AddTexture(aiTextureType_EMISSION_COLOR);
        AddTexture(aiTextureType_METALNESS);
        AddTexture(aiTextureType_DIFFUSE_ROUGHNESS);
        AddTexture(aiTextureType_AMBIENT_OCCLUSION);

        AddTexture(aiTextureType_DIFFUSE, 1);*/

        // Create buffers
        mesh.mIndexBuffer = new IndexBuffer();
        mesh.mVBPosition  = new VertexBuffer();
        mesh.mVBTexcoord  = new VertexBuffer();
        mesh.mVBNormal    = new VertexBuffer();
        mesh.mVBTangent   = new VertexBuffer();
        mesh.mReferenced  = false;

        mesh.mVBPosition->SetSRV(mMeshSRV);
        mesh.mVBTexcoord->SetSRV(mMeshSRV);
        mesh.mVBTangent->SetSRV(mMeshSRV);
        mesh.mVBNormal->SetSRV(mMeshSRV);

        mesh.mIndexBuffer->SetSRV(mMeshSRV);

        mesh.mVBPosition->CreateDefault(Position.size(), sizeof(float3), &Position[0]);
        mesh.mVBTexcoord->CreateDefault(Texcoord.size(), sizeof(float2), &Texcoord[0]);
        mesh.mVBTangent->CreateDefault( Tangent .size(), sizeof(float3), &Tangent [0]);
        mesh.mVBNormal->CreateDefault(  Normal  .size(), sizeof(float3), &Normal  [0]);

        mesh.mIndexBuffer->CreateDefault(IndexNum, &Index[0]);
        
        // Add material
        MatList->push_back(mat);

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

        cbWorldLight = new ConstantBuffer();
        cbWorldLight->CreateDefault(sizeof(WorldLightBuff));
        cbWorldLight->SetName("[Scene::WorldLight]: ConstantBuffer");

        gVelocityIntegrationSystem        = new VelocityIntegrationSystem;
        gAnimatedMeshRenderSystem         = new AnimatedMeshRenderSystem;
        gStaticMeshRenderSystem           = new StaticMeshRenderSystem;
        gMovementControlIntegrationSystem = new MovementControlIntegrationSystem;

        sbPointLightStaticBuffer = new StructuredBuffer<PointLightBuff>();
        sbPointLightStaticBuffer->CreateDefault(256, nullptr, false, D3D11_CPU_ACCESS_WRITE);

        sbPointLightDynamicBuffer = new StructuredBuffer<PointLightBuff>();
        sbPointLightDynamicBuffer->CreateDefault(256, nullptr, false, D3D11_CPU_ACCESS_WRITE);

        // 
        ResetLists();
    }

    ~Scene() {
        SAFE_RELEASE(sbPointLightStaticBuffer);
        SAFE_RELEASE(sbPointLightDynamicBuffer);

        for( auto cb : cbCameraData )
            if( cb ) {
                cb->Release();
                delete cb;
            }

        for( auto cd : mCameraData )
            if( cd ) delete cd;

        SAFE_RELEASE(mSkybox);

        //SAFE_RELEASE_N(cbCameraData, SCENE_MAX_CAMERA_COUNT);
        SAFE_RELEASE(cbTransform);
        SAFE_RELEASE(cbMaterial);
        SAFE_RELEASE(cbAmbientLight);
        SAFE_RELEASE(cbWorldLight);

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
                printf_s("[Scene::~Scene]: Error occured during unloading %s mesh %p\n", name.data(), e);
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
            TransformComponent* transf_ref       = GetComponent<TransformComponent>(e);
            MaterialComponent* mat_ref           = GetComponent<MaterialComponent>(e);

            MeshStaticComponent static_mesh{};
            MeshAnimatedComponent anim_mesh{};
            TransformComponent transf{};
            MaterialComponent mat{};

            // Copy transform data
            memcpy((void*)&transf.mWorld, (void*)&transf_ref->mWorld, sizeof(TransformBuff));

            // Copy material data
            memcpy((void*)&mat._IsTransparent, (void*)&mat_ref->_IsTransparent, sizeof(MaterialBuff));
            memcpy((void*)&mat._AlbedoTex, (void*)&mat_ref->_AlbedoTex, sizeof(uint64_t) * 8 * 2);

            EntityHandle ent = NULL_HANDLE;
            if( static_mesh_ref != NULL_HANDLE ) {
                static_mesh.mIndexBuffer = static_mesh_ref->mIndexBuffer;
                static_mesh.mVBPosition  = static_mesh_ref->mVBPosition;
                static_mesh.mVBTexcoord  = static_mesh_ref->mVBTexcoord;
                static_mesh.mVBTangent   = static_mesh_ref->mVBTangent;
                static_mesh.mVBNormal    = static_mesh_ref->mVBNormal;
                static_mesh.mReferenced  = true;

                ent = mECS.MakeEntity(transf, static_mesh, mat);
            } else if( anim_mesh_ref != NULL_HANDLE ) {
                anim_mesh.mIndexBuffer = anim_mesh_ref->mIndexBuffer;
                anim_mesh.mVBPosition  = anim_mesh_ref->mVBPosition;
                anim_mesh.mVBTexcoord  = anim_mesh_ref->mVBTexcoord;
                anim_mesh.mVBNormal    = anim_mesh_ref->mVBNormal;
                anim_mesh.mVBTangent   = anim_mesh_ref->mVBTangent;
                anim_mesh.mVBWeights   = anim_mesh_ref->mVBWeights;
                anim_mesh.mVBJoints    = anim_mesh_ref->mVBJoints;
                anim_mesh.mReferenced  = true;

                ent = mECS.MakeEntity(transf, anim_mesh, mat);
            }

            if( ent != NULL_HANDLE ) {
                new_list.push_back(ent);
                if( mat_ref->_IsTransparent ) mTransparent.push_back(ent);
                else                          mOpaque.push_back(ent);
            }
        }

        return new_list;
    }

    void AmbientLight(float3 color, float strengh=1.f) {
        AmbientLightComponent* comp = GetComponent<AmbientLightComponent>(mAmbientLight);
        comp->_AmbientLightColor = color;
        comp->_AmbientLightStrengh = strengh;
    }

    void WorldLight(float3 Pos, float3 Dir, float3 Color) {
        // Update
        TransformComponent *td = GetCamera(1)->cTransf;
        td->vPosition = Pos;
        td->vRotation = Dir;

        // Re-build view matrix
        GetCamera(1)->BuildView();

        // Get direction
        mfloat4x4 mInvView = GetCamera(1)->cCam->mInvView;
        float4x4 mInvViewF;
        DirectX::XMStoreFloat4x4(&mInvViewF, mInvView);

        // TODO: Get Rid of Roll Pitch Yaw
        WorldLightComponent* comp  = GetComponent<WorldLightComponent>(mWorldLight);
        comp->_WorldLightPosition  = Pos;
        comp->_WorldLightDirection = { mInvViewF.m[2][0], mInvViewF.m[2][1], mInvViewF.m[2][2] };
        comp->_WorldLightColor     = Color;

    }

    void BindAmbientLight(Shader::ShaderType type, UINT slot=1u) {
        AmbientLightComponent* comp = GetComponent<AmbientLightComponent>(mAmbientLight);
        {
            ScopeMapConstantBufferCopy<AmbientLightBuff> q(cbAmbientLight, &comp->_AmbientLightColor);
        }
        
        cbAmbientLight->Bind(type, slot);
    }

    void BindWorldLight(Shader::ShaderType type, UINT slot=1u) {
        WorldLightComponent* comp = GetComponent<WorldLightComponent>(mWorldLight);
        {
            ScopeMapConstantBufferCopy<WorldLightBuff> q(cbWorldLight, &comp->_WorldLightPosition.x);
        }

        cbWorldLight->Bind(type, slot);
    }

    size_t GetStaticPointLightCount() const { return mPointLightStaticList.size(); };
    size_t GetDynamicPointLightCount() const { return mPointLightDynamicList.size(); };

    EntityHandle InsertStaticPointLight(PointLightBuff light) {
        PointLightComponent L{ light };
        EntityHandle E = mECS.MakeEntity(L);
        mPointLightStaticList.push_back(E);
        return E;
    }

    EntityHandle InsertDynamicPointLight(PointLightBuff light) {
        PointLightComponent L{ light };
        EntityHandle E = mECS.MakeEntity(L);
        mPointLightDynamicList.push_back(E);
        return E;
    }

    StructuredBuffer<PointLightBuff>* ListPointLightDynamicBuffer() {
        if( !mPointLightDynamicList.size() ) { return sbPointLightDynamicBuffer; }

        // Collect data
        std::vector<PointLightBuff> lights;
        lights.reserve(mPointLightDynamicList.size());

        uint32_t i = 0;
        for( EntityHandle light : mPointLightDynamicList ) {
            lights.push_back(GetComponent<PointLightComponent>(light)->GetBuff());
            i++;
        }

        // Fill rest with empties
        /*for( uint32_t j = i; j < sbPointLightDynamicBuffer->GetNumber(); j++ )
            lights.push_back({});*/

        // Send new data
        if( gKeyboard->IsDown(VK_F11) ) 
            printf_s("[SEGV]: LOVI SEGFAULT\n");
        
        {
            ScopedMapCopy—ount<PointLightBuff, StructuredBuffer<PointLightBuff>> map(sbPointLightDynamicBuffer, lights.data(), (uint32_t)lights.size());
        }

        return sbPointLightDynamicBuffer;
    }

    StructuredBuffer<PointLightBuff>* ListPointLightStaticBuffer() {
        if( !mPointLightListStaticUpdate ) return sbPointLightStaticBuffer;

        // Collect data
        std::vector<PointLightBuff> lights;
        lights.reserve(mPointLightStaticList.size());
        
        uint32_t i = 0;
        for( EntityHandle light : mPointLightStaticList ) {
            lights.push_back(GetComponent<PointLightComponent>(light)->GetBuff());
            i++;
        }

        // Fill rest with empties
        /*for( uint32_t j = i; j < sbPointLightStaticBuffer->GetNumber(); j++ )
            lights.push_back({});*/

        // Send new data
        {
            ScopedMapCopy—ount<PointLightBuff, StructuredBuffer<PointLightBuff>> map(sbPointLightStaticBuffer, lights.data(), (uint32_t)lights.size());
        }

        mPointLightListStaticUpdate = false;
        return sbPointLightStaticBuffer;
    }

    // TODO: 
    // StructuredBuffer<float4x4> _WorldMatrices[MAX_INSTANCE_NUM];
    // DrawIndexedInstanced
    uint32_t AddOpaqueStaticInstance(EntityHandle handle, mfloat4x4 mWorld) {
        return 0; // Instance id
    }

    void SetSkybox(const char* fname) {
        SAFE_RELEASE(mSkybox);
        mSkybox = new Texture(fname, 0, "[Scene::Env]: Skybox");
    }

    inline Texture *GetSkybox() const { return mSkybox; }

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

    inline void SetMeshSRV(bool SRV) { mMeshSRV = SRV; };
    inline bool GetMeshSRV() const { return mMeshSRV; };

    EntityHandleList LoadModelStatic(const char* fname, ECS* ecs, uint32_t flags=0u) {
        std::string_view ext = path_ext(fname);

        if( ext == "obj" || ext == "dae" || ext == "fbx" || ext == "gltf" ) {
            return LoadModelExternalStatic(fname, ecs, flags);
        }

        // Other file types
        printf_s("[Scene::LoadModel]: Unsupported model format %s\n", ext.data());
        return {};
    }

    EntityHandleList LoadModelStaticOpaque(const char* fname, uint32_t flags=0u, 
                                           void(*Operator)(EntityHandle e, uint32_t index)
                                           =[](EntityHandle e, uint32_t index)->void{}, ECS* ecs=nullptr) {
        EntityHandleList list = LoadModelStatic(fname, !ecs ? &mECS : ecs, flags);
        
        uint32_t i = 0;
        for( auto e : list ) Operator(e, i++);

        AddOpaque(list);
        return list;
    }
    
    EntityHandleList LoadModelStaticTransparent(const char* fname, uint32_t flags=0u) {
        EntityHandleList list = LoadModelStatic(fname, &mECS, flags);
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
