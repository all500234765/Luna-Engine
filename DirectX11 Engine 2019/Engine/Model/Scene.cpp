#include "Scene.h"



///////////////////////////////// 
VelocityIntegrationSystem::VelocityIntegrationSystem(): BaseECSSystem() {
    AddComponentType(TransformComponent::_ID); // 0
    AddComponentType(VelocityComponent::_ID);  // 1
}

void VelocityIntegrationSystem::UpdateComponents(float dt, BaseECSComponent** comp) {
    TransformComponent* transform = (TransformComponent*)comp[0];
    VelocityComponent* velocity = (VelocityComponent*)comp[1];

    // TODO: Fix +=
    transform->vPosition += velocity->vDirection * velocity->fVelocity * dt;
    velocity->fVelocity += velocity->fAcceleration * dt;

    velocity->fAcceleration *= .9f;
    velocity->fVelocity *= .9f;
}

///////////////////////////////// Opaque
StaticMeshRenderSystem::StaticMeshRenderSystem(): BaseECSSystem() {
    AddComponentType(TransformComponent::_ID);  // 0
    AddComponentType(MeshStaticComponent::_ID); // 1
    AddComponentType(MaterialComponent::_ID);   // 2
}

void StaticMeshRenderSystem::UpdateComponents(float dt, BaseECSComponent** comp) {
    TransformComponent* transform = (TransformComponent*)comp[0];
    MeshStaticComponent* mesh = (MeshStaticComponent*)comp[1];
    MaterialComponent* material = (MaterialComponent*)comp[2];

    Scene* scene = Scene::Current();

    // If it's transparency pass, but material is opaque, 
    // or material layer mask doesn't have at least one bit with current renderable material layer
    //      Skip rendering
    if( material->_IsTransparent != scene->IsTransparentPass()
       || (!(material->_MaterialLayer &  scene->GetEnabledMaterialLayers()) && material->_MaterialLayer) ) return;

    // Get flags
    uint32_t flags = ieee_uint32(dt);

    // If material can't cast shadows and currently we are rendering shadow map
    //      Then skip
    if( !material->_ShadowCaster && (flags & RendererFlags::ShadowPass) ) return;

    // Target shader types
    Shader::ShaderType type = Shader::ShaderType(flags >> 25);
    Shader::ShaderType mat_type = Shader::ShaderType((flags >> 18) & 0x7F);

    // Bind mesh data
    mesh->Bind();
    transform->Build();
    transform->Bind(scene->cbTransform, type, 0);
    material->Bind(scene->cbMaterial, mat_type, 0, flags);
    // TODO: Make sorting by materials

    // Draw call1
    gDirectX->gContext->DrawIndexed(mesh->mIndexBuffer->GetNumber(), 0, 0);
}

/*class VelocityIntegrationSystem: public BaseECSSystem {
public:
    VelocityIntegrationSystem(): BaseECSSystem() {
        AddComponentType(TransformComponent::_ID); // 0
        AddComponentType(VelocityComponent::_ID);  // 1
    }

    void UpdateComponents(float dt, BaseECSComponent** comp) override {
        TransformComponent* transform = (TransformComponent*)comp[0];
        VelocityComponent* velocity = (VelocityComponent*)comp[1];

        // TODO: Fix +=
        transform->vPosition += velocity->vDirection * velocity->fVelocity * dt;
        velocity->fVelocity += velocity->fAcceleration * dt;

        velocity->fAcceleration *= .9f;
        velocity->fVelocity *= .9f;
    }

};

class StaticMeshRenderSystem: public BaseECSSystem {
public:
    StaticMeshRenderSystem(): BaseECSSystem() {
        AddComponentType(TransformComponent::_ID);  // 0
        AddComponentType(MeshStaticComponent::_ID); // 1
    }

    void UpdateComponents(float dt, BaseECSComponent** comp) override {
        TransformComponent* transform = (TransformComponent*)comp[0];
        MeshStaticComponent* mesh = (MeshStaticComponent*)comp[1];

        uint32_t flags = ieee_uint32(dt);
        uint32_t low = (flags >> 24);

        uint32_t slot = low >> Shader::Count;
        Shader::ShaderType type = Shader::ShaderType(low & ((1 << Shader::Count) - 1));
        //(type | (slot << Shader::Count));

        mesh->Bind();
        //transform->Build();
        transform->Bind(Scene::Current()->cbTransform, Shader::Vertex, 0u);

        gDirectX->gContext->DrawIndexed(mesh->mIndexBuffer.GetNumber(), 0, 0);
    }
};*/

