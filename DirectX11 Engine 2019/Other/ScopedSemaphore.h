#pragma once

#include <mutex>
#include <condition_variable>

template<uint32_t GroupIndex>
class ScopedSemaphore {
protected:
    struct Group {
        uint32_t gCurrent;
        std::mutex gMutex;
        std::condition_variable gCondVar;
    };

    static Group gGroups[128];

private:
    

public:
    ScopedSemaphore(uint32_t num) {
        std::unique_lock<std::mutex> lock(gGroups[GroupIndex].gMutex);

        while( gGroups[GroupIndex].gCurrent != num ) gGroups[GroupIndex].gCondVar.wait(lock);
    }

    ~ScopedSemaphore() { gGroups[GroupIndex].gCurrent++; NotifyAll(); }

    static void NotifyAll() { gGroups[GroupIndex].gCondVar.notify_all(); }
    static void NotifyOne() { gGroups[GroupIndex].gCondVar.notify_one(); }
    
    static uint32_t GetCurrent() { return gGroups[GroupIndex].gCurrent; };
};

