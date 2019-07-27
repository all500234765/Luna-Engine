#include "ConstantBuffer.h"
#include "Buffer.h"
#include <iostream>

void ConstantBuffer::CreateDefault(UINT size) {
    D3D11_BUFFER_DESC desc;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = size;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    HRESULT hr = gDirectX->gDevice->CreateBuffer(&desc, NULL, &pBuff);
    std::cout << "cBuffer created (error=" << hr << ", size=" << size << ")" << std::endl;
}

void* ConstantBuffer::Map() {
    D3D11_MAPPED_SUBRESOURCE res;
    HRESULT hr;
    
    // Try to get mapped resource
    hr = gDirectX->gContext->Map(pBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
    if( FAILED(hr) ) {
        std::cout << "Can't receive constant buffer." << std::endl;
        return NULL;
    }

    return res.pData;
}

void ConstantBuffer::Unmap() {
    gDirectX->gContext->Unmap(pBuff, 0);
}

void ConstantBuffer::Bind(Shader::ShaderType type, UINT slot) {
    switch( type ) {
        case Shader::Vertex  : gDirectX->gContext->VSSetConstantBuffers(slot, 1, &pBuff); break;
        case Shader::Pixel   : gDirectX->gContext->PSSetConstantBuffers(slot, 1, &pBuff); break;
        case Shader::Geometry: gDirectX->gContext->GSSetConstantBuffers(slot, 1, &pBuff); break;
        case Shader::Hull    : gDirectX->gContext->HSSetConstantBuffers(slot, 1, &pBuff); break;
        case Shader::Domain  : gDirectX->gContext->DSSetConstantBuffers(slot, 1, &pBuff); break;
        case Shader::Compute : gDirectX->gContext->CSSetConstantBuffers(slot, 1, &pBuff); break;
    }
}

void ConstantBuffer::Release() {
    __super::Release();
}
