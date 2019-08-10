#pragma once

#include "DirectX.h"

class DirectXChild {

protected:
    static _DirectX* gDirectX;
    virtual void _SetName(ID3D11Resource *res, const char* name) {
        if( res ) res->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(name) - 1, name);
    }

public:
    static void SetDirectX(_DirectX* dx);
};
