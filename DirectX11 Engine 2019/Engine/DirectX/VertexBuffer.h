#pragma once

#include "pc.h"
#include "Buffer.h"

class VertexBuffer: public Buffer {
private:
    ID3D11ShaderResourceView *pSRV;
    bool bSRV = false;

public:
    inline void SetSRV(bool state) { bSRV = state; }

    inline ID3D11ShaderResourceView* GetSRV() const { return pSRV; }
    
    void CreateDefault(size_t Num, size_t _Stride, void* vertices, bool staging=false, DXGI_FORMAT format=DXGI_FORMAT_UNKNOWN);

    inline void Release() {
        if( pSRV ) pSRV->Release();
        if( pBuff ) pBuff->Release();
    }
};
