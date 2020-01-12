#pragma once

#include "pc.h"
#include "DirectX/DirectX.h"

extern _DirectX* gDirectX;

#include "Engine/DirectX/ConstantBuffer.h"
#include "Engine/DirectX/VertexBuffer.h"
#include "Engine/DirectX/IndexBuffer.h"

template<typename T, class S>
class ScopedMap {
private:
    S* obj;

public:
    T* data;

    ScopedMap(S* o): obj(o) { data = (T*)obj->Map(); }
    ~ScopedMap() { obj->Unmap(); };
};

template<typename T>
class ScopedMapResource {
private:
    ID3D11Resource* obj;
    HRESULT hr;

public:
    T* data;

    ScopedMapResource(ID3D11Resource* o): obj(o) {
        D3D11_MAPPED_SUBRESOURCE res;
        gDirectX->gContext->Map(obj, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
        data = (T*)res.pData;
    }

    ~ScopedMapResource() { gDirectX->gContext->Unmap(obj, 0); };
};

template<typename T, class S>
class ScopedMapCopy {
private:
    S* obj;

public:
    T* data;

    template<typename T2>
    ScopedMapCopy(S* o, T2  * src): obj(o) { data = (T*)obj->Map(); memcpy(&data[0], (void*)&src [0], sizeof(T)); }
    ScopedMapCopy(S* o, void* src): obj(o) { data = (T*)obj->Map(); memcpy(&data[0], &((T*)  src)[0], sizeof(T)); }
    ScopedMapCopy(S* o, T   * src): obj(o) { data = (T*)obj->Map(); memcpy(&data[0], &       src [0], sizeof(T)); }

    ~ScopedMapCopy() { obj->Unmap(); };
};

////////////////////////////////////////////////////////////////////////////
// Constant buffer
////////////////////////////////////////////////////////////////////////////
// Map CB & Unmap at the end of scope
template<typename T>
using ScopeMapConstantBuffer = ScopedMap<T, ConstantBuffer>;

// Copy data to CB & unmap at the end of scope
template<typename T>
using ScopeMapConstantBufferCopy = ScopedMapCopy<T, ConstantBuffer>;

////////////////////////////////////////////////////////////////////////////
// Vertex Buffer
////////////////////////////////////////////////////////////////////////////
// Copy data to VB & unmap at the end of scope
template<typename T>
using ScopeMapVertexBuffer = ScopedMap<T, VertexBuffer>;

// Copy data to VB & unmap at the end of scope
template<typename T>
using ScopeMapVertexBufferCopy = ScopedMapCopy<T, VertexBuffer>;

////////////////////////////////////////////////////////////////////////////
// Index Buffer
////////////////////////////////////////////////////////////////////////////
// Copy data to IB & unmap at the end of scope
using ScopeMapIndexBuffer = ScopedMap<uint, IndexBuffer>;

// Copy data to IB & unmap at the end of scope
using ScopeMapIndexBufferCopy = ScopedMapCopy<uint, IndexBuffer>;

////////////////////////////////////////////////////////////////////////////
// 
////////////////////////////////////////////////////////////////////////////

//template<typename T, typename Type>
//using ScopeMapStructuredBuffer = ScopedMap<T, class StructuredBuffer<Type>>;

// TODO: 
//template<typename T>
//using ScopeMapTexture          = ScopedMap<T, class Texture>;

/*
Usage example:
    struct DataBuffer { float b; };
    class Something* cbDataBuffer;
    
    template<typename T>
    using ScopeMapSomething = ScopedMap<T, Something>;
    
    int main()
    {
        ScopeMapSomething<DataBuffer> q(cbDataBuffer);
    
        q.data->b = 1.f;
        
    }
*/
