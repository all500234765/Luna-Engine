#pragma once

#include "DirectX/DirectX.h"

extern _DirectX* gDirectX;

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

class ConstantBuffer;

template<typename T>
using ScopeMapConstantBuffer   = ScopedMap<T, ConstantBuffer>;

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
