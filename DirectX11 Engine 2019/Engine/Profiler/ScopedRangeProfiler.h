#pragma once

#include "pc.h"
#include "RangeProfiler.h"

class ScopedRangeProfiler {
public:
    ScopedRangeProfiler(const char* ScopeName) {
        RangeProfiler::Begin(ScopeName);
    }

    ScopedRangeProfiler(const wchar_t* ScopeName) {
        RangeProfiler::Begin(ScopeName);
    }

    ScopedRangeProfiler() {
        RangeProfiler::Begin("Unnamed scope");
    }

    ~ScopedRangeProfiler() {
        RangeProfiler::End();
    }
};

class ScopedRangeGPUProfiler {
private:
    float* pDest{};
    const char* mScopeName{};
    bool mPrint{};

public:
    ScopedRangeGPUProfiler(const char* ScopeName) : mScopeName(ScopeName), mPrint(true) {
        RangeProfilerGPU::Begin();
    }
    
    ScopedRangeGPUProfiler(float* dest) {
        RangeProfilerGPU::Begin();
    }

    ~ScopedRangeGPUProfiler() {
        RangeProfilerGPU::End();

        if( mPrint ) {
            float dest = RangeProfilerGPU::GetTime();
            printf_s("[%s]: %fms\n", mScopeName, dest);
        } else {
            *pDest = RangeProfilerGPU::GetTime();
        }

    }
};
