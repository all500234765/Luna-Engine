#pragma once

#include "Buffer.h"
#include "Shader.h"

template<typename T>
class StructuredBuffer: public Buffer {
    ID3D11ShaderResourceView *pSRV;

public:
    void CreateDefault(UINT num, T* data) {
        HRESULT hr = S_OK;
        Stride = sizeof(T);
        Number = num;

        // Create buffer
        D3D11_BUFFER_DESC pDesc = {};
        pDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE;
        pDesc.ByteWidth           = Number * Stride;
        pDesc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        pDesc.Usage               = D3D11_USAGE_DEFAULT;
        pDesc.CPUAccessFlags      = 0;
        pDesc.StructureByteStride = Stride;

        D3D11_SUBRESOURCE_DATA pData = {};
        pData.pSysMem = &data[0];
        pData.SysMemPitch = 0;
        pData.SysMemSlicePitch = 0;

        if( (hr = gDirectX->gDevice->CreateBuffer(&pDesc, &pData, &pBuff)) != S_OK ) {
            std::cout << "Failed to create StructuredBuffer (error=" << hr << ")" << std::endl;
            return;
        }

        std::cout << "Successfully created StructuredBuffer (size=" << Number * Stride << ", num=" << Number << ")" << std::endl;

        // Create SRV
        D3D11_SHADER_RESOURCE_VIEW_DESC pSRVDesc = {};
        pSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        pSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
        pSRVDesc.Buffer.FirstElement = 0;
        pSRVDesc.Buffer.NumElements = Number;

        if( (hr = gDirectX->gDevice->CreateShaderResourceView(pBuff, &pSRVDesc, &pSRV)) != S_OK ) {
            std::cout << "Failed to create SRV for StructuredBuffer (error=" << hr << ")" << std::endl;
            return;
        }

        std::cout << "Successfully created SRV for StructuredBuffer" << std::endl;
    }

    void Bind(Shader::ShaderType type, UINT slot){
        if( !pSRV ) { return; }
        switch( type ) {
            case Shader::Vertex  : gDirectX->gContext->VSSetShaderResources(slot, 1, &pSRV); break;
            case Shader::Pixel   : gDirectX->gContext->PSSetShaderResources(slot, 1, &pSRV); break;
            case Shader::Geometry: gDirectX->gContext->GSSetShaderResources(slot, 1, &pSRV); break;
            case Shader::Hull    : gDirectX->gContext->HSSetShaderResources(slot, 1, &pSRV); break;
            case Shader::Domain  : gDirectX->gContext->DSSetShaderResources(slot, 1, &pSRV); break;
            case Shader::Compute : gDirectX->gContext->CSSetShaderResources(slot, 1, &pSRV); break;
        }
    }

    void Release() {
        if( pSRV ) pSRV->Release();
        if( pBuff ) pBuff->Release();
    }
};
