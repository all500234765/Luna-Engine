#include "PhysicsObject.h"

void PhysicsObject::Integrate(float dt) {
    mPosition += mVelocity * dt;
}

CollisionData PhysicsObject::Collide(const PhysicsObjectSphere& other) const {
    return CollisionData();
}

CollisionData PhysicsObject::Collide(const PhysicsObjectPlane& other) const {
    return CollisionData();
}

CollisionData PhysicsObject::Collide(const PhysicsObjectAABB& other) const {
    return CollisionData();
}
