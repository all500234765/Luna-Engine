#pragma once

#include "Engine/DirectX/IndexBuffer.h"
#include "Engine/DirectX/VertexBuffer.h"
#include "Engine/DirectX/ConstantBuffer.h"

#include "VertexSets.h"

template<uint32_t NumVertexSets>
class NewMesh: public DirectXChild {
private:
    IndexBuffer mIndexBuffer{};
    VertexBuffer mVertexBuffer[NumVertexBuffers]{};
    UINT pStrides[NumVertexSets]{};

public:
    NewMesh();
    ~NewMesh() {
        if( mIndexBuffer ) mIndexBuffer->Release();
        for( uint32_t i = 0; i < NumVertexSets; i++ ) if( mVertexBuffer[i] ) mVertexBuffer[i]->Release();
    }

    void SetIndexBuffer(IndexBuffer&& buff) { mIndexBuffer = std::move(buff); }
    void SetVertexBuffers(VertexBuffer& buffs) {
        mVertexBuffer = buffs;

        UINT stride = 0u;
        for( uint32_t i = 0u; i < NumVertexSets; i++ ) {
            stride += mVertexBuffer[i].GetStride();
            pStrides[i] = stride;
        }
    }

    inline void Bind(const UINT* pOffsets=nullptr) const {
        gDirectX->gContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32, 0);
        gDirectX->gContext->IASetVertexBuffers(0, NumVertexSets, &mVertexBuffer, pStrides, pOffsets);
    }

};
