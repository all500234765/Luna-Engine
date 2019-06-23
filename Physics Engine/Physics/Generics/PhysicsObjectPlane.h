#pragma once

#include "PhysicsObject.h"

#include "PhysicsObjectSphere.h"
#include "PhysicsObjectAABB.h"

class PhysicsObjectPlane: public PhysicsObject {
private:
    pFloat3 mNormal;
    pFloat mDistance;

public:
    PhysicsObjectPlane(const pFloat3& normal, pFloat dist): mNormal(normal), mDistance(dist) {};

    inline const pFloat3& GetNormal() const { return mNormal; }
    inline pFloat GetDistance() const { return mDistance; }

    const PhysicsObjectPlane& Normalized() const;

    CollisionData Collide(const PhysicsObjectSphere& other) const override;
    CollisionData Collide(const PhysicsObjectPlane& other) const override;
    CollisionData Collide(const PhysicsObjectAABB& other) const override;

};
