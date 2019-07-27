#pragma once

#include "DirectXChild.h"

class Buffer: public DirectXChild {
private:

protected:
    D3D11_BUFFER_DESC pDesc;
    ID3D11Buffer *pBuff;
    UINT Stride, Offset, Number;

    void Create(D3D11_BUFFER_DESC desc, D3D11_SUBRESOURCE_DATA data, UINT stride, UINT offset);

public:
    virtual void Release();
    ID3D11Buffer* GetBuffer();
    UINT GetNumber();
    void BindVertex(UINT Slot);
    void BindIndex(UINT Off, DXGI_FORMAT Format);
};
