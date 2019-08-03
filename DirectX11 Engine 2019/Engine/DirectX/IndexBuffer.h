#pragma once

#include "Buffer.h"

class IndexBuffer: public Buffer {
public:
    void CreateDefault(UINT Num, void* indices);

};
