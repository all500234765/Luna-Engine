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

CollisionData PhysicsObject::Collide(const PhysicsObject& other) const {
    switch( mType ) {
        case PhysicsShapeType::Sphere  : return Collide((const PhysicsObjectSphere&)other);
        case PhysicsShapeType::AABB    : return Collide((const PhysicsObjectAABB&)other);
        case PhysicsShapeType::Plane   : return Collide((const PhysicsObjectPlane&)other);
        //case PhysicsShapeType::Box     : return Collide((const PhysicsObjectBox&)other);
        //case PhysicsShapeType::Cylinder: return Collide((const PhysicsObjectCylinder&)other);
        //case PhysicsShapeType::Complex : return Collide((const PhysicsObjectComplex&)other);
    }

    return CollisionData();
}
