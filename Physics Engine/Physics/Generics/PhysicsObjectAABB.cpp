#include "PhysicsObjectAABB.h"

CollisionData PhysicsObjectAABB::Collide(const PhysicsObjectSphere& other) const {
    return CollisionData();
}

CollisionData PhysicsObjectAABB::Collide(const PhysicsObjectPlane& other) const {
    return CollisionData();
}

CollisionData PhysicsObjectAABB::Collide(const PhysicsObjectAABB& other) const {
    pFloat dist = ((pFloat3)other.GetMinPos() - mMaxPos).max(
                   (pFloat3)mMinPos - (pFloat3)other.GetMaxPos()).max();

    return { (dist < 0), dist };
}
