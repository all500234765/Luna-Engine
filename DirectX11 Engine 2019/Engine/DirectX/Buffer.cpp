#include "Buffer.h"

#include <iostream>

void Buffer::Create(D3D11_BUFFER_DESC desc, D3D11_SUBRESOURCE_DATA data, UINT stride, UINT offset) {
    pDesc = desc;
    Stride = stride;
    Offset = offset;
    Number = desc.ByteWidth / stride;

    HRESULT hr = gDirectX->gDevice->CreateBuffer(&desc, &data, &pBuff);

    std::cout << "Buffer created (error=" << hr << ")" << std::endl;
}

void Buffer::Release() {
    pBuff->Release();
    pBuff = 0;
}

ID3D11Buffer* Buffer::GetBuffer() {
    return pBuff;
}

UINT Buffer::GetNumber() {
    return Number;
}

void Buffer::BindVertex(UINT Slot) {
    // Set the vertex buffer to active in the input assembler so it can be rendered.
    gDirectX->gContext->IASetVertexBuffers(Slot, 1, &pBuff, &Stride, &Offset);
}

void Buffer::BindIndex(UINT Off, DXGI_FORMAT Format) {
    // Set the index buffer to active in the input assembler so it can be rendered.
    gDirectX->gContext->IASetIndexBuffer(pBuff, Format, Off);
}
