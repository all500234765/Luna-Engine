#include "DirectXChild.h"

void DirectXChild::SetDirectX(_DirectX* dx) {
    gDirectX = dx;
}

void DirectXChild::SetName(const char* name) {
    gDirectX->gContext->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(name) - 1, name);
}

_DirectX* DirectXChild::gDirectX = 0;
