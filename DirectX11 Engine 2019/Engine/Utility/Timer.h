#pragma once

#include <chrono>
#include "TimerLog.h"

class Timer: public TimerLog {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> mStart;
    const char* mName = "Unknown";

public:
    Timer() { StartOver(); }
    Timer(const char* name): mName(name) { StartOver(); }

    ~Timer() { Stop(); }

    void StartOver() {
        mStart = std::chrono::high_resolution_clock::now();

    }

    void Stop() {
        auto mEnd = std::chrono::high_resolution_clock::now();

        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(mStart).time_since_epoch().count();
        auto end   = std::chrono::time_point_cast<std::chrono::microseconds>(mEnd  ).time_since_epoch().count();

        auto dur = end - start;

        Log(mName, dur);
    }

};
