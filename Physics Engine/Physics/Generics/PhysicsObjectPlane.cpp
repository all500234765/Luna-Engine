#include "PhysicsObjectPlane.h"

const PhysicsObjectPlane& PhysicsObjectPlane::Normalized() const {
    pFloat len = mNormal.length();
    return PhysicsObjectPlane(mNormal / len, mDistance / len);
}

CollisionData PhysicsObjectPlane::Collide(const PhysicsObjectSphere& other) const {
    float dFsc = mDistance + mNormal.dot(other.GetPosition()); // Distance from sphere center
    float dFs = dFsc - other.GetRadius(); // Distance from sphere

    return { dFs < 0, dFs };
}

CollisionData PhysicsObjectPlane::Collide(const PhysicsObjectPlane& other) const {
    return CollisionData();
}

CollisionData PhysicsObjectPlane::Collide(const PhysicsObjectAABB& other) const {
    return CollisionData();
}
