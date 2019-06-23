#pragma once

#include "..\PhysicsTypes.h"

struct CollisionData {
private:
    const bool mResult;
    const pFloat mDist;
    //const pFloat mNormal;

public:
    CollisionData(): mResult(false), mDist(0) {};
    CollisionData(const bool res, const float dist): mResult(res), mDist(dist) {};

    inline bool GetResult() const { return mResult; }
    inline pFloat GetDist() const { return mDist; }
};
