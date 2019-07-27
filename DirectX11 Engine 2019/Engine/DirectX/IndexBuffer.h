#pragma once

#include "Buffer.h"
#include <d3d11.h>

class IndexBuffer: public Buffer {
public:
    void CreateDefault(UINT Num, void* indices);
    void Release() override;

};
