#pragma once

#include "PhysicsObject.h"

#include "PhysicsObjectSphere.h"
#include "PhysicsObjectPlane.h"

class PhysicsObjectAABB: public PhysicsObject {
private:
    pFloat3 mMinPos;
    pFloat3 mMaxPos;

public:
    PhysicsObjectAABB(PhysicsCollider* col);

    inline const pFloat3& GetMinPos() const { return mMinPos; }
    inline const pFloat3& GetMaxPos() const { return mMaxPos; }

};
