#include "DirectXChild.h"

//#include "Profiling.h"
#include "Engine/Profiler/RangeProfiler.h"

void DirectXChild::SetDirectX(_DirectX* dx) {
    gDirectX = dx;

    RangeProfiler::Init();
}

_DirectX* DirectXChild::gDirectX = 0;
