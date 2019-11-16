#pragma once

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