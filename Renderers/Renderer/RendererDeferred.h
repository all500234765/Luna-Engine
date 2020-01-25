#pragma once

#include "pc.h"
#include "RendererBase.h"

class RendererDeferred: public RendererBase {
private:
    // Defaults
    struct {
        union {
            float4 black_void1[4];
            const float black_void2[4] = { 0.f, 0.f, 0.f, 0.f };
        };

        union {
            float4 white_snow1[4];
            const float white_snow2[4] = { 1.f, 1.f, 1.f, 1.f };
        };


    } s_clear;

    struct {
        union {
            Texture* texture_list[8];

            struct {
                Texture *checkboard{};
                Texture *tile_normal{};
                Texture *bluenoise_rg_512{};
                Texture *black{};
                Texture *white{};
                Texture *mgray{}; // Middle gray; 127
                Texture *dgray{}; // Dark gray; 63
                Texture *lgray{}; // Light gray; 190
            } tex{};
        } ti{};

        struct {
            SamplerState point{};
            SamplerState linear{};
            SamplerState point_comp{};
            SamplerState linear_comp{};
        } sampl{};

        // TODO: Do clustered rendering?
        Texture *mCubemap{};

    } s_material{};

    struct {
        struct {
            BlendState *normal;
            BlendState *add;
            BlendState *no_blend;
        } blend;

        struct {
            // Depth test/write, no stencil
            DepthStencilState *normal;
            DepthStencilState *norw;   // No RW
            DepthStencilState *ro;     // Read Only
        } depth;

        struct {
            RasterState *normal;
            RasterState *wire;
            RasterState *normal_scissors;
            RasterState *wire_scissors;
            RasterState *normal_cfront;
            RasterState *normal_cback;
        } raster;
    } s_states{};

    struct {
        MeshComponent unit_sphere;

        void Release() {
            SAFE_RELEASE(unit_sphere.mIndexBuffer);
            SAFE_RELEASE(unit_sphere.mVBPosition );
            SAFE_RELEASE(unit_sphere.mVBTangent  );
            SAFE_RELEASE(unit_sphere.mVBTexcoord );
        }

#define GET_BUFFER(x) ((x) ? (x)->GetBuffer() : nullptr)
#define GET_STRIDE(x) ((x) ? (x)->GetStride() : 0u)

        void Bind(const MeshComponent& mesh) {
            ID3D11Buffer *buffs[4] = { GET_BUFFER(mesh.mVBPosition), GET_BUFFER(mesh.mVBTexcoord), 
                                       GET_BUFFER(mesh.mVBNormal),   GET_BUFFER(mesh.mVBTangent) };

            UINT strides[4] = { GET_STRIDE(mesh.mVBPosition), GET_STRIDE(mesh.mVBTexcoord), 
                                GET_STRIDE(mesh.mVBNormal),   GET_STRIDE(mesh.mVBTangent) };
            UINT offsets[4] = { 0, 0, 0, 0 };

            gDirectX->gContext->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, strides, offsets);
            gDirectX->gContext->IASetIndexBuffer(mesh.mIndexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
        }

#undef GET_BUFFER
#undef GET_STRIDE
    } s_mesh{};

    // Deferred renderer
    ConstantBuffer *cbDeferredGlobal;
    struct DeferredGlobal {
        #include "Deferred/Global.h"
    };

    // Volumetric lights
    ConstantBuffer *cbVolumetricSettings;
    struct VolumetricSettings {
        //                              fQ = fFar / (fNear - fFar);
        float4 _ProjValues;  // fNear * fQ, fQ, 1 / m[0][0], 1 / m[1][1] // Player
        float4 _ProjValues2; //                                          // Light
        float2 _Scaling;     // Width, Height / Downscaling Factor
        float _GScattering;  // [-1; 1]
        float _MaxDistance;  // 0 - Light Far?
        uint _FrameIndex;    // 0 -> Interleaved; based on frame index
        uint _Interleaved;   // pow(2, n)
        float _Exposure;     // Default: 10.f
        uint _Padding;
    };

    // Blur filter
    ConstantBuffer *cbBlurFilter;
    struct BlurFilter {
        // Res of downscaled target: x - width, y - height
        uint2 _Res; // Backbuffer / 4

        // Total pixels in the downscaled img
        uint _Domain; // Res.x * Res.y

        // Number of groups dispatched on 1st pass
        uint _GroupSize; // Domain / 1024

        // 
        float4 _Alignment;
    } gBlurFilter{};

    // DSSDO
    ConstantBuffer *cbDSSDOSettings;
    struct DSSDOSettings {
        //                              fQ = fFar / (fNear - fFar);
        float4 _ProjValues;  // fNear * fQ, fQ, 1 / m[0][0], 1 / m[1][1] // Player
        float2 _Scaling;     // Width, Height / Downscaling Factor
        uint _FrameIndex;    // 0 -> Interleaved; based on frame index
        uint _Interleaved;   // pow(2, n)
        float _OcclusionRadius;
        float _OcclusionMaxDistance;
        uint2 _Padding;
    };

    // Effects
    HDRPostProcess                 *gHDRPostProcess{};
    SSAOPostProcess                *gSSAOPostProcess{};
    SSLRPostProcess                *gSSLRPostProcess{};
    SSLFPostProcess                *gSSLFPostProcess{};
    CascadeShadowMapping<3, false> *gCascadeShadowMapping{};
    //CoverageBuffer                 *gCoverageBuffer{};
    OrderIndendentTransparency     *gOrderIndendentTransparency{};

    SSLRArgs    gSSLRArgs{};
    SSAOArgs    gSSAOArgs{};
    CSMArgs     gCSMArgs{};
    //CBuffArgs   gCBuffArgs{};
    OITSettings gOITSettings{};
    VolumetricSettings gVolumetricSettings{};
    DSSDOSettings gDSSDOSettings{};

    // Resources
    RenderTarget2DDepthMSAA       *rtDepth{};
    RenderTarget2DColor5DepthMSAA *rtGBuffer{};
    RenderTarget2DColor4DepthMSAA *rtTransparency{}, *rtCombinedGBuffer{};
    RenderTarget2DColor1          *rtFinalPass{};
    RenderTarget2DColor1DepthMSAA *rtDeferred{};
    RenderTarget2DColor1          *rtDeferredAccumulation{};

    Texture *mVolumetricLightAccum{};
    Texture *mDepth2{}, *mDepthI{}; // Main depth buffer & Intermidiate
    Texture *mDSSDOAccumulation{};

    Shader *shSurface{}, *shVertexOnly{}, *shGUI{}, *shPostProcess{}, *shCombinationPass{};
    Shader *shDeferredPointLights{}, *shDeferredAccumulation{};
    Shader *shVolumetricLight;
    Shader *shSimpleGUI{};
    Shader *shHorizontalFilterDepth{}, *shVerticalFilterDepth{};
    Shader *shDSSDOAccumulate{};
    
    // Local
    Scene *mScene{};

    // Transformation
    TransformComponent *IdentityTransf;
    ConstantBuffer *cbTransform;

    // 
    uint gFrameIndex{};

    // ImGui
    bool mRenderDoc{};
    ImTextureID mRenderDocImageID{};
    Texture *mRenderDocTex{};

    enum class LitIndex {
        Lit, Unlit, 
        AO, Indirect, 
        Volumetric,
        SSDO,
        Normal,
        Deferred, 
        DeferredA,  // Accumulation
        Shading,

        Count
    };

    bool mLit{};
    uint32_t mLitIndex{};
    ImTextureID mLitImageID[(uint)LitIndex::Count]{};
    Texture *mLitImageTex[(uint)LitIndex::Count]{};

    // 
    template<typename T>
    struct LightDescriptor {
        uint num;
        StructuredBuffer<T>* sb;
    };

    // Geometry Passes
    void Shadows();             // Done
    void GBuffer();             // Done
    void OIT();                 // Done; Optimize sorting; Fix MSAA

    // Occlusion tests
    void CoverageBuffer();

    // Screen-Space Passes
    void Deferred();
    void DeferredLights(const LightDescriptor<PointLightBuff>& point);
    
    void SSAO();
    void SSLR();
    void SSLF();
    void FSSSSS();
    void Combine();
    void HDR();
    void VolumetricLight();
    void DSSDO();

    // Final Passes
    void Final();

    void BindOrtho();

    // Internal mesh loading
    
    TransformComponent DefaultTransformComp() const {
        TransformComponent transform{};
        transform.mWorld    = DirectX::XMMatrixIdentity();
        transform.vPosition = { 0.f, 0.f, 0.f };
        transform.vRotation = { 0.f, 0.f, 0.f };
        transform.vScale    = { 1.f, 1.f, 1.f };

        return transform;
    }

    uint32_t mMeshSRV{};
    std::vector<MeshComponent> LoadModelExternal(const char* fname, /*ECS* ecs, */uint32_t flags) {

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
        std::vector<MeshComponent> mMeshList;
        //std::vector<MaterialComponent> mMatList;
        //LoaderTextureList mTextureFileList;

        uint32_t index = 0u;
        ProcessNodeStatic(scene->mRootNode, scene, &mMeshList, /*&mMatList, &mTextureFileList, */index);

        // Get current model's directory
        /*char drive[_MAX_DRIVE];
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
        }*/

        // Done
        return mMeshList;

        //if( mMeshList.size() == 1 ) {
        //    return { ecs->MakeEntity(transform, mMeshList[0], mMatList[0]) };
        //}
        //
        //// Return list of entites
        //EntityHandleList list;
        //list.reserve(mMeshList.size());
        //for( uint32_t i = 0; i < mMeshList.size(); i++ ) {
        //    list.push_back(ecs->MakeEntity(transform, mMeshList[i], mMatList[i]));
        //}
        //
        //// Done
        //return list;
    }

    void ProcessNodeStatic(aiNode* node, const aiScene* scene, std::vector<MeshComponent>* MeshList,
                           //std::vector<MaterialComponent>* MatList, 
                           //LoaderTextureList* TextureList,
                           uint32_t& index) {
        // Process meshes
#pragma omp parallel for num_threads(4)
        for( int32_t i = 0; i < (int32_t)node->mNumMeshes; i++ ) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            MeshList->push_back(ProcessMeshStatic(mesh, scene, /*MatList, TextureList, */index));
            index++;
        }

        // Process children
        for( size_t i = 0; i < node->mNumChildren; i++ ) {
            ProcessNodeStatic(node->mChildren[i], scene, MeshList, /*MatList, TextureList, */index);
        }
    }

    MeshComponent ProcessMeshStatic(aiMesh* inMesh, const aiScene* scene, 
                           //std::vector<MaterialComponent>* MatList, 
                           //LoaderTextureList* TextureList, 
                           uint32_t index) {
        //MaterialComponent mat = DefaultMaterialComp();
        MeshComponent mesh{};

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
        /*uint32_t mat_index = inMesh->mMaterialIndex;
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
            printf_s(" - %u:%u\n", type, m->GetTextureCount(type));

            // No texture found
            if( !strcmp(fname.C_Str(), "") ) return false;
            LoaderTextureList::iterator it = TextureList->find(s);

            if( it == TextureList->end() ) {
                printf_s("%s; %u\n", s.c_str(), type);

                // Add
                TextureList->insert_or_assign(s, LoaderTextureList::mapped_type({ aiTextureType(type + i * (AI_TEXTURE_TYPE_MAX + 1)), { index } }));
                return true;
            }

            TextureList->at(s).second.push_back(index);
            return true;
        };

        printf_s("-------------\n");

        /*AddTexture(aiTextureType_DIFFUSE  ); // Albedo
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
        mesh.mVBTangent->CreateDefault( Tangent.size() , sizeof(float3), &Tangent [0]);
        mesh.mVBNormal->CreateDefault(  Normal  .size(), sizeof(float3), &Normal  [0]);

        mesh.mIndexBuffer->CreateDefault(IndexNum, &Index[0]);
        
        // Add material
        //MatList->push_back(mat);

        // Done
        return mesh;
    }

    MeshComponent LoadMeshInternal(const char* fname) {
        std::ifstream file(fname, std::ios::binary);

        if( !file.is_open() ) {
            printf_s("[RendererDeferred::LoadMeshInternal]: Failed to open file %s.\n", fname);
            return {};
        }

        // "VB P/N/UV; I_N=" + Index Num
        // Position
        // Normal
        // UV
        struct Header {
            uint8_t V, B, Space;

            uint8_t P, G0;
            uint8_t N, G1;
            uint8_t UV, G2;
            uint8_t G3;
            uint8_t I_N_[5];
        } head{};

        file.read((char*)&head, sizeof(Header));
        
        uint8_t x[10];
        uint32_t IndexNum = 0u, e = 0u;
        for( uint32_t i = 0; i < 10; i++ ) {
            file.read((char*)&x[i], 1);
            if( x[i] == '|' ) { break; }
            e++;
        }

        // 256
        //  2 * 10^2 | 2 - (2 - 0)
        // +5 * 10^1 | 2 - (2 - 1)
        // +6 * 10^0 | 2 - (2 - 2)
        // e=3

        for( uint32_t i = 0; i < e; i++ ) {
            IndexNum += (uint32_t)powf(10.f, (e - 1) - i) * (x[i] - '0');
        }

        // Load mesh data
        std::vector<float3> Position, Normal;
        std::vector<float2> Texcoord;
        std::vector<uint32_t> Index;

        Index.reserve(IndexNum);

        //Position.reserve(IndexNum * 3);
        //Normal.reserve(IndexNum * 3);
        //Texcoord.reserve(IndexNum * 2);

        Position.resize(IndexNum * 3);
        Normal.resize(IndexNum * 3);
        Texcoord.resize(IndexNum * 3);
        
        file.read((char*)Position.data(), IndexNum * sizeof(float3) * 3);
        file.read((char*)Normal  .data(), IndexNum * sizeof(float3) * 3);
        file.read((char*)Texcoord.data(), IndexNum * sizeof(float2) * 3);

        //std::copy(std::istream_iterator<float3>(file), std::istream_iterator<float3>(file) + IndexNum * 3, std::back_inserter(Position));
        //std::copy(std::istream_iterator<float3>(file), std::istream_iterator<float3>(file) + IndexNum * 3, std::back_inserter(Normal  ));
        //std::copy(std::istream_iterator<float2>(file), std::istream_iterator<float2>(file) + IndexNum * 2, std::back_inserter(Texcoord));

        file.close();

        // Create indices
        for( uint32_t i = 0; i < IndexNum; i++ )
            Index.push_back(i);

        // Create mesh
        MeshComponent mesh{};
        
        // Create buffers
        mesh.mIndexBuffer = new IndexBuffer();
        mesh.mVBPosition  = new VertexBuffer();
        mesh.mVBTexcoord  = new VertexBuffer();
        mesh.mVBNormal    = new VertexBuffer();
        //mesh.mVBTangent   = new VertexBuffer();
        mesh.mReferenced  = false;

        mesh.mVBPosition->SetSRV(mMeshSRV);
        mesh.mVBTexcoord->SetSRV(mMeshSRV);
        //mesh.mVBTangent->SetSRV(mMeshSRV);
        mesh.mVBNormal->SetSRV(mMeshSRV);

        mesh.mIndexBuffer->SetSRV(mMeshSRV);

        mesh.mVBPosition->CreateDefault(Position.size(), sizeof(float3), &Position[0]);
        mesh.mVBTexcoord->CreateDefault(Texcoord.size(), sizeof(float2), &Texcoord[0]);
        //mesh.mVBTangent->CreateDefault( Tangent.size() , sizeof(float3), &Tangent [0]);
        mesh.mVBNormal->CreateDefault(  Normal  .size(), sizeof(float3), &Normal  [0]);

        mesh.mIndexBuffer->CreateDefault(IndexNum, &Index[0]);
        
        return mesh;
    }
public:
    RendererDeferred(): RendererBase() {};
    ~RendererDeferred() {
        printf_s("[~RendererDeferred]\n");
    };

    virtual void Init()        override;
    virtual void Resize()      override;
    virtual void Render()      override;
    virtual void FinalScreen() override;
    virtual void Release()     override;
    virtual void ImGui()       override;
    virtual void ClearMainRT() override;
    virtual void DebugHUD()    override;

    //inline Texture* GetTexture(uint32_t index) const override { return s_material.texture_list[index]; };
    inline Texture* GetTexture(TextureList index) const { return s_material.ti.texture_list[(uint32_t)index]; };
};
