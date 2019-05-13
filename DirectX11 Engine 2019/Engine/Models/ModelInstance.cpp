#include "ModelInstance.h"

void ModelInstance::SetModel(Model* model) {
    mModel = model;
}

void ModelInstance::SetShader(Shader* shader) {
    shShader = shader;
}

void ModelInstance::SetTopology(D3D11_PRIMITIVE_TOPOLOGY pTopology) {
    Topology = pTopology;
}

void ModelInstance::SetBindBuffer(Shader::ShaderType BB, UINT Slot) {
    BindBuffer = BB;
    BindBufferSlot = Slot;
}

void ModelInstance::SetWorldMatrix(DirectX::XMMATRIX mWorld) {
    World = mWorld;
}

void ModelInstance::Bind(Camera* cam) {
    shShader->Bind();
    gDirectX->gContext->IASetPrimitiveTopology(Topology);

    cam->SetWorldMatrix(World);
    cam->BindBuffer(BindBuffer, BindBufferSlot);
    cam->BuildConstantBuffer();
}

void ModelInstance::Render() {
    mModel->Render();
}

void ModelInstance::Render(UINT Num) {
    mModel->Render(Num);
}
