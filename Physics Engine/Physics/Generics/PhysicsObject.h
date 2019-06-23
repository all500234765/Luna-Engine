#pragma once

class PhysicsObjectSphere;
class PhysicsObjectPlane;
class PhysicsObjectAABB;

#include "../PhysicsTypes.h"
#include "Physics/Collision/CollisionData.h"

typedef enum: char {
    Undefined,
    Sphere,
    AABB,
    Plane,
    Box,
    Cylinder,
    Complex
} PhysicsShapeType;

class PhysicsObject {
private:
    PhysicsShapeType mType = PhysicsShapeType::Undefined;
    pFloat3 mVelocity;
    pFloat3 mPosition;
    //pQuat   mOrientation;

public:
    PhysicsObject(): mPosition(), mVelocity() {};
    PhysicsObject(const pFloat3& pos, const pFloat3& vel): mPosition(pos), mVelocity(vel) {};

    inline const pFloat3& GetVelocity()  const { return mVelocity; }
    inline const pFloat3& GetPosition()  const { return mPosition; }
    //inline const pQuat& GetOrientation() const { return mOrientation; }

    inline void SetVelocity(const pFloat3& vel) { mVelocity = vel; }
    inline void SetPosition(const pFloat3& pos) { mPosition = pos; }

    // Shape
    inline PhysicsShapeType GetShapeType() const { return mType; }
    inline void SetType(PhysicsShapeType type) { mType = type; }

    // Integration
    void Integrate(float dt);

    // Collisions
    virtual CollisionData Collide(const PhysicsObjectSphere& other) const;
    virtual CollisionData Collide(const PhysicsObjectPlane& other) const;
    virtual CollisionData Collide(const PhysicsObjectAABB& other) const;
    CollisionData Collide(const PhysicsObject& other) const;
};
