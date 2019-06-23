#include "PhysicsObjectSphere.h"

PhysicsObjectSphere::PhysicsObjectSphere(const pFloat3& center, float rad, const pFloat3& vel): mRadius(rad) {
    SetVelocity(vel);
    SetPosition(center);
    SetType(PhysicsShapeType::Sphere);
}

CollisionData PhysicsObjectSphere::Collide(const PhysicsObjectSphere& other) const {
    pFloat rDist = mRadius + other.mRadius;
    pFloat cDist = ((pFloat3)other.GetPosition() - GetPosition()).length();

    // Check for intersection
    return { (cDist < rDist), cDist - rDist };
}

CollisionData PhysicsObjectSphere::Collide(const PhysicsObjectPlane& other) const {
    return CollisionData();
}

CollisionData PhysicsObjectSphere::Collide(const PhysicsObjectAABB& other) const {
    return CollisionData();
}
