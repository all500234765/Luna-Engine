#pragma once

#include "Buffer.h"

class VertexBuffer: public Buffer {
private:
    ID3D11ShaderResourceView *pSRV;
    bool bSRV = false;

public:
    inline void SetSRV(bool state) { bSRV = state; }

    inline ID3D11ShaderResourceView* GetSRV() const { return pSRV; }
    
    void CreateDefault(UINT Num, UINT _Stride, void* vertices);

    inline void Release() {
        if( pSRV ) pSRV->Release();
        if( pBuff ) pBuff->Release();
    }
};
