#pragma once

#include "pc.h"
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
        desc.MiscFlags = (bSRV ? D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS : 0);
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
            pSRVDesc.ViewDimension         = D3D11_SRV_DIMENSION_BUFFEREX;
            pSRVDesc.Format                = DXGI_FORMAT_R32_TYPELESS;
            pSRVDesc.BufferEx.FirstElement = 0;
            pSRVDesc.BufferEx.NumElements  = Number;
            pSRVDesc.BufferEx.Flags        = D3D11_BUFFEREX_SRV_FLAG_RAW;

            if( (hr = gDirectX->gDevice->CreateShaderResourceView(pBuff, &pSRVDesc, &pSRV)) != S_OK ) {
                printf_s("[IB]: Failed to create SRV. (error=%d)\n", hr);
                return;
            }
            
            printf_s("[IB]: Successfully created SRV\n");
        }
    }

    void Release() {
        if( pSRV ) pSRV->Release();
        if( pBuff ) pBuff->Release();
    }

};
