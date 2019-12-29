#pragma once

#include "pc.h"
#include "Model.h"
#include "Engine/DirectX/Shader.h"
#include "Engine/Camera/Camera.h"

class ModelInstance: public DirectXChild {
private:
    Model  *mModel;
    Shader *shShader;

    DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();
    D3D11_PRIMITIVE_TOPOLOGY Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    Shader::ShaderType BindBuffer = Shader::Vertex;
    UINT BindBufferSlot = 0;

public:
    inline void SetModel(Model* model) { mModel = model; }
    inline void SetShader(Shader* shader) { shShader = shader; }

    inline D3D11_PRIMITIVE_TOPOLOGY GetTopology() const { return Topology; }
    inline DirectX::XMMATRIX GetWorldMatrix() const { return World; }

    // Optional
    inline void SetTopology(D3D11_PRIMITIVE_TOPOLOGY pTopology)   { Topology = pTopology; }
    inline void SetBindBuffer(Shader::ShaderType BB, UINT Slot=0) { BindBuffer = BB; BindBufferSlot = Slot; }
    inline void SetWorldMatrix(DirectX::XMMATRIX mWorld)          { World = mWorld; }

    void Bind(Camera* cam) {
        shShader->Bind();
        gDirectX->gContext->IASetPrimitiveTopology(Topology);

        cam->SetWorldMatrix(World);
        cam->BuildConstantBuffer();
        cam->BindBuffer(BindBuffer, BindBufferSlot);
    }

    inline void Render(bool bBindTextures=true) { mModel->Render(1, bBindTextures); }
    inline void Render(UINT Num, bool bBindTextures=true) { mModel->Render(Num, bBindTextures); }
};
