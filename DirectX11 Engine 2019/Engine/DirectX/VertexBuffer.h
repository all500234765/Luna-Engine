#pragma once

#include "Buffer.h"
#include <d3d11.h>

class VertexBuffer: public Buffer {
public:
    void CreateDefault(UINT Num, UINT _Stride, void* vertices);
    void Release() override;

};
