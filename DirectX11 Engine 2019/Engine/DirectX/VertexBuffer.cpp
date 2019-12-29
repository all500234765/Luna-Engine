#include "pc.h"
#include "VertexBuffer.h"

void VertexBuffer::CreateDefault(UINT Num, UINT _Stride, void* vertices) {
    // Create default index buffer
    D3D11_BUFFER_DESC desc;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = Num * _Stride;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER | (bSRV ? D3D11_BIND_SHADER_RESOURCE : 0);
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = vertices;
    data.SysMemPitch = _Stride*0;
    data.SysMemSlicePitch = 0*desc.ByteWidth;

    // 
    Create(desc, data, _Stride, 0);

    // Create Shader Resource View
    if( bSRV ) {
        HRESULT hr = S_OK;
        D3D11_SHADER_RESOURCE_VIEW_DESC pSRVDesc = {};
        pSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        pSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
        pSRVDesc.Buffer.FirstElement = 0;
        pSRVDesc.Buffer.NumElements = Number;

        if( (hr = gDirectX->gDevice->CreateShaderResourceView(pBuff, &pSRVDesc, &pSRV)) != S_OK ) {
            printf_s("Failed to create SRV for VertexBuffer (error=%d)", hr);
            return;
        }

        printf_s("Successfully created SRV for VertexBuffer");
    }
}
