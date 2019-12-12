#pragma once

#include <thread>

template<uint32_t N>
class ScopedThreadSync {
private:
    std::thread mThreads[N];

public:
    ScopedThreadSync(void(*func)(uint32_t id, uint32_t max_threads)) {
        for( uint32_t id = 0; id < N; id++ ) mThreads[id] = std::thread(func, id, N);
    }

    ~ScopedThreadSync() {
        for( uint32_t id = 0; id < N; id++ ) mThreads[id].join();
    }
};

