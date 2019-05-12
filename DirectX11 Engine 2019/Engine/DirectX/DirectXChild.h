#pragma once

#include "DirectX.h"

class DirectXChild {

protected:
    static _DirectX* gDirectX;

public:
    static void SetDirectX(_DirectX* dx);
    void SetName(const char* name);
};
