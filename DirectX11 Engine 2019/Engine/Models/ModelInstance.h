#pragma once

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
    void SetModel(Model* model);
    void SetShader(Shader* shader);

    inline D3D11_PRIMITIVE_TOPOLOGY GetTopology() const { return Topology; }
    inline DirectX::XMMATRIX GetWorldMatrix() const { return World; }

    // Optional
    void SetTopology(D3D11_PRIMITIVE_TOPOLOGY pTopology);
    void SetBindBuffer(Shader::ShaderType BB, UINT Slot=0);
    void SetWorldMatrix(DirectX::XMMATRIX mWorld);

    void Bind(Camera* cam);
    void Render(bool bBindTextures=true);
    void Render(UINT Num, bool bBindTextures=true);
};
