#pragma once

#include "Buffer.h"

class IndexBuffer: public Buffer {
private:
    ID3D11ShaderResourceView *pSRV;
    bool bSRV = false;

public:
    inline void SetSRV(bool state) { bSRV = state; }

    inline ID3D11ShaderResourceView* GetSRV() const { return pSRV; }

    void CreateDefault(UINT Num, void* indices) {
        // Create default index buffer
        D3D11_BUFFER_DESC desc;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.ByteWidth = sizeof(unsigned long) * Num;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER | (bSRV ? D3D11_BIND_SHADER_RESOURCE : 0);
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA data;
        data.pSysMem = indices;
        data.SysMemPitch = 0;
        data.SysMemSlicePitch = 0;

        // 
        Create(desc, data, sizeof(unsigned long), 0);

        // Create Shader Resource View
        if( bSRV ) {
            HRESULT hr = S_OK;
            D3D11_SHADER_RESOURCE_VIEW_DESC pSRVDesc = {};
            pSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
            pSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
            pSRVDesc.Buffer.FirstElement = 0;
            pSRVDesc.Buffer.NumElements = Number;

            if( (hr = gDirectX->gDevice->CreateShaderResourceView(pBuff, &pSRVDesc, &pSRV)) != S_OK ) {
                printf_s("Failed to create SRV for IndexBuffer (error=%d)", hr);
                return;
            }
            
            printf_s("Successfully created SRV for IndexBuffer");
        }
    }

    void Release() {
        if( pSRV ) pSRV->Release();
        if( pBuff ) pBuff->Release();
    }

};
