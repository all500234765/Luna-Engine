#pragma once

#include "PhysicsObject.h"

#include "PhysicsObjectAABB.h"
#include "PhysicsObjectPlane.h"

class PhysicsObjectSphere: public PhysicsObject {
private:
    pFloat3 mPosition;
    pFloat mRadius;

public:
    PhysicsObjectSphere(const pFloat3& center, float rad, const pFloat3& vel);

    inline pFloat GetRadius() const { return mRadius; }
    inline const pFloat3& GetPosition() const { return mPosition; }

    CollisionData Collide(const PhysicsObjectSphere& other) const override;
    CollisionData Collide(const PhysicsObjectPlane& other) const override;
    CollisionData Collide(const PhysicsObjectAABB& other) const override;

};
