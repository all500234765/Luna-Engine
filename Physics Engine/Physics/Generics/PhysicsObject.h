#pragma once

class PhysicsObjectSphere;
class PhysicsObjectPlane;
class PhysicsObjectAABB;

#include "../PhysicsTypes.h"
#include "Physics/Collision/CollisionData.h"

class PhysicsObject {
private:
    pFloat3 mVelocity;
    pFloat3 mPosition;
    //pQuat   mOrientation;
    
public:
    PhysicsObject(): mPosition(), mVelocity() {};
    PhysicsObject(const pFloat3& pos, const pFloat3& vel): mPosition(pos), mVelocity(vel) {};

    inline const pFloat3& GetVelocity() const { return mVelocity; }
    inline const pFloat3& GetPosition() const { return mPosition; }
    //inline const pQuat& GetOrientation() const { return mOrientation; }

    inline void SetVelocity(const pFloat3& vel) { mVelocity = vel; }

    void Integrate(float dt);

    virtual CollisionData Collide(const PhysicsObjectSphere& other) const;
    virtual CollisionData Collide(const PhysicsObjectPlane& other) const;
    virtual CollisionData Collide(const PhysicsObjectAABB& other) const;
};
