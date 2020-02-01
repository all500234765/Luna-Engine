#include "pc.h"
#include "VertexBuffer.h"

void VertexBuffer::CreateDefault(size_t Num, size_t _Stride, void* vertices, bool staging, DXGI_FORMAT format) {
    // Create default index buffer
    D3D11_BUFFER_DESC desc{};
    desc.Usage               = staging ? D3D11_USAGE_STAGING : (bSRV ? D3D11_USAGE_DEFAULT : D3D11_USAGE_IMMUTABLE);
    desc.ByteWidth           = (UINT)(Num * _Stride);
    desc.BindFlags           = staging ? 0 : (D3D11_BIND_VERTEX_BUFFER | (bSRV ? D3D11_BIND_SHADER_RESOURCE : 0));
    desc.CPUAccessFlags      = staging ? D3D11_CPU_ACCESS_READ : 0;
    desc.MiscFlags           = staging ? 0 : (bSRV ? D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS : 0);
    desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem          = vertices;
    data.SysMemPitch      = (UINT)_Stride;
    data.SysMemSlicePitch = desc.ByteWidth;

    // 
    Create(desc, data, (UINT)_Stride, 0);

    // Create Shader Resource View
    if( bSRV ) {
        HRESULT hr = S_OK;
        D3D11_SHADER_RESOURCE_VIEW_DESC pSRVDesc = {};
        pSRVDesc.ViewDimension         = D3D11_SRV_DIMENSION_BUFFER;
        pSRVDesc.Format                = format;
        pSRVDesc.Buffer.FirstElement   = 0;
        pSRVDesc.Buffer.NumElements    = Number;

        if( (hr = gDirectX->gDevice->CreateShaderResourceView(pBuff, &pSRVDesc, &pSRV)) != S_OK ) {
            printf_s("[VB]: Failed to create SRV. (error=%d)\n", hr);
            return;
        }

        printf_s("[VB]: Successfully created SRV\n");
    }
}
