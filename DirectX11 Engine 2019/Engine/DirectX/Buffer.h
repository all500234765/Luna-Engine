#pragma once

#include "pc.h"
#include "DirectXChild.h"

class Buffer: public DirectXChild {
protected:
    D3D11_BUFFER_DESC pDesc;
    ID3D11Buffer *pBuff;
    UINT Stride, Offset, Number;

    virtual inline void Create(D3D11_BUFFER_DESC desc, D3D11_SUBRESOURCE_DATA data, UINT stride, UINT offset) {
        pDesc = desc;
        Stride = stride;
        Offset = offset;
        Number = desc.ByteWidth / stride;

        HRESULT hr = gDirectX->gDevice->CreateBuffer(&desc, &data, &pBuff);

        //std::cout << "Buffer created (error=" << hr << ")" << std::endl;
    }

public:
    virtual inline ID3D11Buffer* GetBuffer() const { return pBuff; }
    virtual inline UINT GetNumber() const          { return Number; }
    virtual inline void Release()                  { if( pBuff ) pBuff->Release(); pBuff = 0; }

    // Set the vertex buffer to active in the input assembler so it can be rendered.
    virtual inline void BindVertex(UINT Slot=0) {
        gDirectX->gContext->IASetVertexBuffers(Slot, 1, &pBuff, &Stride, &Offset);
    }

    // Set the index buffer to active in the input assembler so it can be rendered.
    virtual inline void BindIndex(UINT Off, DXGI_FORMAT Format) {
        gDirectX->gContext->IASetIndexBuffer(pBuff, Format, Off);
    }

    virtual inline void SetName(const char* name) { _SetName(pBuff, name); }
    virtual inline UINT GetStride() const { return Stride; }
};
