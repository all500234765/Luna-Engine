#pragma once

#include "DirectX.h"
#include <stdio.h>

class DirectXChild {

protected:
    static _DirectX* gDirectX;
    virtual void _SetName(ID3D11Resource *res, const char* name) {
        if( res ) {
            res->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(name), name);
            printf_s("[DXC]: Set name for %p [%s]\n", res, name);
        }
    }

public:
    static void SetDirectX(_DirectX* dx);
};
