#pragma once

#include "PhysicsObject.h"

#include "PhysicsObjectSphere.h"
#include "PhysicsObjectAABB.h"

class PhysicsObjectPlane: public PhysicsObject {
private:
    pFloat3 mNormal;
    pFloat mDistance;

public:
    PhysicsObjectPlane(PhysicsCollider* col);

    inline const pFloat3& GetNormal() const { return mNormal; }
    inline void SetNormal(const pFloat3& n) { mNormal = n; }

    inline pFloat GetDistance() const { return mDistance; }
    inline void SetDistance(pFloat d) { mDistance = d; }

    const PhysicsObjectPlane& Normalized() const;
};
