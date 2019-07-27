#include "IndexBuffer.h"
#include "Buffer.h"
#include <d3d11.h>

void IndexBuffer::CreateDefault(UINT Num, void* indices) {
    // Create default index buffer
    D3D11_BUFFER_DESC desc;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = sizeof(unsigned long) * Num;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = indices;
    data.SysMemPitch = 0;
    data.SysMemSlicePitch = 0;

    // 
    __super::Create(desc, data, sizeof(unsigned long), 0);
}

void IndexBuffer::Release() {
    __super::Release();
}
