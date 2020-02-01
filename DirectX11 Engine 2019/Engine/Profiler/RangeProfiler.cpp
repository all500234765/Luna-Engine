#include "pc.h"
#include "RangeProfiler.h"

ID3DUserDefinedAnnotation *RangeProfiler::gUDAnnotation = nullptr;
bool RangeProfiler::gInitialized = false;

std::array<Query*, gRangeProfilerStackSize> RangeProfilerGPU::gQStackDisjoint{};
std::array<Query*, gRangeProfilerStackSize> RangeProfilerGPU::gQStackTimestampBegin{};
std::array<Query*, gRangeProfilerStackSize> RangeProfilerGPU::gQStackTimestampEnd{};
std::array<UINT64, gRangeProfilerStackSize> RangeProfilerGPU::gStackTime{};
uint RangeProfilerGPU::gStackPos{};

