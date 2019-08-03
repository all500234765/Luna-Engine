#include "VertexBuffer.h"

void VertexBuffer::CreateDefault(UINT Num, UINT _Stride, void* vertices) {
    // Create default index buffer
    D3D11_BUFFER_DESC desc;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = Num * _Stride;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = vertices;
    data.SysMemPitch = _Stride*0;
    data.SysMemSlicePitch = 0*desc.ByteWidth;

    // 
    Create(desc, data, _Stride, 0);
}
