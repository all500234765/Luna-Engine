#pragma once

#include "Buffer.h"

class VertexBuffer: public Buffer {
public:
    void CreateDefault(UINT Num, UINT _Stride, void* vertices);

};
