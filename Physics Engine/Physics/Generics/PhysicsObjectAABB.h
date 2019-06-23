#pragma once

#include "PhysicsObject.h"

#include "PhysicsObjectSphere.h"
#include "PhysicsObjectPlane.h"

class PhysicsObjectAABB: public PhysicsObject {
private:
    pFloat3 mMinPos;
    pFloat3 mMaxPos;

public:
    PhysicsObjectAABB(const pFloat3& min, const pFloat3& max): mMinPos(min), mMaxPos(max) {
        SetType(PhysicsShapeType::AABB);
    };

    inline const pFloat3& GetMinPos() const { return mMinPos; }
    inline const pFloat3& GetMaxPos() const { return mMaxPos; }

    CollisionData Collide(const PhysicsObjectSphere& other) const override;
    CollisionData Collide(const PhysicsObjectPlane& other) const override;
    CollisionData Collide(const PhysicsObjectAABB& other) const override;

};
