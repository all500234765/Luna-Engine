#pragma once

#include "PhysicsCollider.h"
#include "PhysicsObject.h"

class PhysicsObjectSphere: public PhysicsObject {
private:
    pFloat mRadius;

public:
    PhysicsObjectSphere(PhysicsCollider* col);

    inline pFloat GetRadius() const { return mRadius; }
    inline void SetRadius(pFloat r) { mRadius = r; }
};
