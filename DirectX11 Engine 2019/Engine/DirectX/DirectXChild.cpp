#include "DirectXChild.h"

void DirectXChild::SetDirectX(_DirectX* dx) {
    gDirectX = dx;
}

_DirectX* DirectXChild::gDirectX = 0;
