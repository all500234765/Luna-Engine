#pragma once

// Might be very dungerous!
#include "pc.h"
#include "Engine/Extensions/Safe.h"

template<typename T>
class StaticReference {
protected:
    static std::vector<T> gStaticReferenceList;

public:
    static void AddStaticRef(T *ref) {
        gStaticReferenceList.push_back(ref);
    }

    static void StaticRelease() {
        for( T* e : gStaticReferenceList )
            SAFE_RELEASE(e);

        gStaticReferenceList.clear();
    }
};

template<typename T>
std::vector<T> StaticReference<T>::gStaticReferenceList = {};
