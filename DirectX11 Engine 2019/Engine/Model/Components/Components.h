struct VelocityBuff {
    #include "Velocity.h"
};

struct TransformBuff {
    #include "Transform.h"
};

struct CameraBuff {
    #include "Camera.h"
};

struct MaterialBuff {
    #include "Material.h"
};

struct VelocityComponent: ECSComponent<VelocityComponent> {
    #include "Velocity.h"
};

struct TransformComponent: ECSComponent<TransformComponent> {
    #include "Transform.h"

    void Bind(ConstantBuffer* cb, uint32_t types, uint32_t slot) {
        {
            // Update CB
            ScopeMapConstantBufferCopy<TransformBuff> q(cb, (void*)&this->mWorld);
        }

        cb->Bind(types, slot);
    }
};

struct CameraComponent: ECSComponent<CameraComponent> {
    #include "Camera.h"

    void Bind(ConstantBuffer* cb, uint32_t types, uint32_t slot) {
        {
            // Update CB
            ScopeMapConstantBufferCopy<CameraBuff> q(cb, (void*)&this->mView);
        }

        cb->Bind(types, slot);
    }
};

struct MaterialComponent: ECSComponent<MaterialComponent> {
    #include "Material.h"

    Texture2D _AlbedoTex           TEXSLOT(0);
    Texture2D _NormalTex           TEXSLOT(1);
    Texture2D _MetallicTex         TEXSLOT(2);
    Texture2D _RougnessTex         TEXSLOT(3);
    Texture2D _EmissionTex         TEXSLOT(4);
    Texture2D _AmbientOcclusionTex TEXSLOT(5);

    SamplerState _AlbedoSampl           SAMPLSLOT(0);
    SamplerState _NormalSampl           SAMPLSLOT(1);
    SamplerState _MetallicSampl         SAMPLSLOT(2);
    SamplerState _RougnessSampl         SAMPLSLOT(2);
    SamplerState _EmissionSampl         SAMPLSLOT(3);
    SamplerState _AmbientOcclusionSampl SAMPLSLOT(4);
    
    void Bind(ConstantBuffer* cb, uint32_t types, uint32_t slot) {
        {
            // Update CB
            ScopeMapConstantBufferCopy<MaterialBuff> q(cb, (void*)&this->_IsTransparent);
        }

        cb->Bind(types, slot);
    }
};

// Mesh
struct MeshComponent {
    VertexBuffer *mVBPosition;
    VertexBuffer *mVBTexcoord;
    VertexBuffer *mVBNormal;
    //VertexBuffer *mVBQuaternion; // TODO: GPU Pro 3; Replace tangent and bitangent with quats

    IndexBuffer *mIndexBuffer;
};

struct MeshStaticComponent: ECSComponent<MeshStaticComponent>, MeshComponent {
    const char* mFname;


    void Bind() {
        ID3D11Buffer *buffs[3] = { mVBPosition->GetBuffer(), 
                                   mVBTexcoord->GetBuffer(), 
                                   mVBNormal->GetBuffer() };

        UINT strides[3] = { mVBPosition->GetStride(), 
                            mVBTexcoord->GetStride(), 
                            mVBNormal->GetStride() };
        UINT offsets[3] = { 0, 0, 0 };

        gDirectX->gContext->IASetVertexBuffers(0, 3, buffs, strides, offsets);
        gDirectX->gContext->IASetIndexBuffer(mIndexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
    }
};

struct MeshAnimatedComponent: ECSComponent<MeshAnimatedComponent>, MeshComponent {
    VertexBuffer *mVBWeights;
    VertexBuffer *mVBJoints;
    
    void Bind() {
        ID3D11Buffer *buffs[5] = { mVBPosition->GetBuffer(), 
                                   mVBTexcoord->GetBuffer(), 
                                   mVBNormal->GetBuffer(),
                                   mVBWeights->GetBuffer(),
                                   mVBJoints->GetBuffer() };

        UINT strides[5] = { mVBPosition->GetStride(), mVBTexcoord->GetStride(), 
                            mVBNormal->GetStride(), mVBWeights->GetStride(), 
                            mVBJoints->GetStride() };
        UINT offsets[5] = { 0, 0, 0, 0, 0 };

        gDirectX->gContext->IASetVertexBuffers(0, 5, buffs, strides, offsets);
        gDirectX->gContext->IASetIndexBuffer(mIndexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
    }
};

// 
/*struct: ECSComponent<> {

};

struct : ECSComponent<> {

};*/
