#pragma once

#include "..\PhysicsTypes.h"

struct CollisionData {
private:
    const bool mResult;
    const pFloat3 mNormal;

public:
    CollisionData(): mResult(false), mNormal({}) {};
    CollisionData(const bool res, const pFloat3& n): mResult(res), mNormal(n) {};

    inline bool GetResult() const { return mResult; }
    inline pFloat GetDist() const { return mNormal.length(); }
    inline const pFloat3& GetNormal() const { return mNormal; }
};
