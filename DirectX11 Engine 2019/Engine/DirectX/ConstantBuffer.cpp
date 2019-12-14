#include "ConstantBuffer.h"
#include <iostream>

void ConstantBuffer::CreateDefault(UINT size) {
    pDesc.Usage = D3D11_USAGE_DYNAMIC;
    pDesc.ByteWidth = size;
    pDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    pDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    pDesc.MiscFlags = 0;
    pDesc.StructureByteStride = 0;

    HRESULT hr = gDirectX->gDevice->CreateBuffer(&pDesc, NULL, &pBuff);
    std::cout << "ConstantBuffer created (error=" << hr << ", size=" << size << ")" << std::endl;
}

void* ConstantBuffer::Map(D3D11_MAP map) {
    if( !pBuff ) { return nullptr; }

    D3D11_MAPPED_SUBRESOURCE res;
    HRESULT hr;

    // Try to get mapped resource
    hr = gDirectX->gContext->Map(pBuff, 0, map, 0, &res);
    if( FAILED(hr) ) {
        std::cout << "Can't map ConstantBuffer." << std::endl;
        return NULL;
    }

    return res.pData;
}

void ConstantBuffer::Unmap() {
    if( !pBuff ) { return; }
    gDirectX->gContext->Unmap(pBuff, 0);
}

void ConstantBuffer::Bind(UINT type, UINT slot) {
    if( !pBuff ) { return; }

    if( type & Shader::Vertex   ) gDirectX->gContext->VSSetConstantBuffers(slot, 1, &pBuff);
    if( type & Shader::Pixel    ) gDirectX->gContext->PSSetConstantBuffers(slot, 1, &pBuff);
    if( type & Shader::Geometry ) gDirectX->gContext->GSSetConstantBuffers(slot, 1, &pBuff);
    if( type & Shader::Hull     ) gDirectX->gContext->HSSetConstantBuffers(slot, 1, &pBuff);
    if( type & Shader::Domain   ) gDirectX->gContext->DSSetConstantBuffers(slot, 1, &pBuff);
    if( type & Shader::Compute  ) gDirectX->gContext->CSSetConstantBuffers(slot, 1, &pBuff);
}
