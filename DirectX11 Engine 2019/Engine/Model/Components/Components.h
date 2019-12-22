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

struct WorldLightBuff {
    #include "WorldLight.h"
};

struct AmbientLightBuff {
    #include "AmbientLight.h"
};

struct AmbientLightComponent: ECSComponent<AmbientLightComponent> {
    #include "AmbientLight.h"

    void Bind(ConstantBuffer* cb, uint32_t types, uint32_t slot) {
        {
            // Update CB
            ScopeMapConstantBufferCopy<AmbientLightBuff> q(cb, (void*)&this->_AmbientLightColor);
        }

        cb->Bind(types, slot);
    }
};

struct WorldLightComponent: ECSComponent<WorldLightComponent> {
    #include "WorldLight.h"

    void Bind(ConstantBuffer* cb, uint32_t types, uint32_t slot) {
        {
            // Update CB
            ScopeMapConstantBufferCopy<WorldLightBuff> q(cb, (void*)&this->_WorldLightPosition);
        }

        cb->Bind(types, slot);
    }
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

    void Build() {
        mWorld = DirectX::XMMatrixIdentity();

        mWorld *= DirectX::XMMatrixRotationRollPitchYaw(vRotation.x, vRotation.y, vRotation.z);
        mWorld *= DirectX::XMMatrixScaling(vScale.x, vScale.y, vScale.z);
        mWorld *= DirectX::XMMatrixTranslation(vPosition.x, vPosition.y, vPosition.z);
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
    #include "MaterialTextures.h"

    void Bind(ConstantBuffer* cb, uint32_t types, uint32_t slot, uint32_t flags) {
        if( (flags & RendererFlags::DepthPass) == 0 ) {
            _Alb   = _AlbedoTex           != nullptr;
            _Metal = _MetallicTex         != nullptr;
            _Rough = _RoughnessTex        != nullptr;
            _Emis  = _EmissionTex         != nullptr;
            _AO    = _AmbientOcclusionTex != nullptr;

        }

        {
            // Update CB
            ScopeMapConstantBufferCopy<MaterialBuff> q(cb, (void*)&this->_IsTransparent);
        }

        // Material params
        cb->Bind(types, slot);

        // Textures
        if( flags & RendererFlags::DontBindTextures ) return;
        Shader::ShaderType type = Shader::ShaderType(types);

        if( _AlbedoTex ) _AlbedoTex->Bind(type, 0);
        if( (flags & RendererFlags::DontBindSamplers) == 0 && _AlbedoSampl ) _AlbedoSampl->Bind(type, 0);

        if( (flags & RendererFlags::DepthPass) == 0 ) {
            if( _NormalTex           ) _NormalTex          ->Bind(type, 1);
            if( _MetallicTex         ) _MetallicTex        ->Bind(type, 2);
            if( _RoughnessTex        ) _RoughnessTex       ->Bind(type, 3);
            if( _EmissionTex         ) _EmissionTex        ->Bind(type, 4);
            if( _AmbientOcclusionTex ) _AmbientOcclusionTex->Bind(type, 5);

            if( flags & RendererFlags::DontBindSamplers ) return;
            if( _AlbedoSampl           ) _AlbedoSampl          ->Bind(type, 0);
            if( _NormalSampl           ) _NormalSampl          ->Bind(type, 1);
            if( _MetallicSampl         ) _MetallicSampl        ->Bind(type, 2);
            if( _RoughnessSampl        ) _RoughnessSampl       ->Bind(type, 3);
            if( _EmissionSampl         ) _EmissionSampl        ->Bind(type, 4);
            if( _AmbientOcclusionSampl ) _AmbientOcclusionSampl->Bind(type, 5);
        }
    }
};

// Mesh
struct MeshComponent {
    VertexBuffer *mVBPosition;
    VertexBuffer *mVBTexcoord;
    VertexBuffer *mVBNormal;
    //VertexBuffer *mVBQuaternion; // TODO: GPU Pro 3; Replace tangent and bitangent with quats

    IndexBuffer *mIndexBuffer;

    uint32_t mReferenced;
};

struct MeshStaticComponent: ECSComponent<MeshStaticComponent>, MeshComponent {


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