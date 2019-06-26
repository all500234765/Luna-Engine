#pragma once

#include "PhysicsCollider.h"
#include "../PhysicsTypes.h"

class PhysicsObject {
private:
    pFloat3 mVelocity;
    pFloat3 mPosition;
    //pQuat   mOrientation;
    PhysicsCollider* mCollider;
    bool mFixed;
    pFloat mMass;

public:
    PhysicsObject(): mPosition(), mVelocity() {};
    PhysicsObject(const pFloat3& pos, const pFloat3& vel): mPosition(pos), mVelocity(vel) {};

    ~PhysicsObject() {
        if( mCollider->GetRef()->RemoveReference() ) {
            delete mCollider;
        }
    };

    inline const pFloat3& GetVelocity() const { return mVelocity; }
    inline const pFloat3& GetPosition() const { return mPosition; }
    //inline const pQuat& GetOrientation() const { return mOrientation; }

    inline void SetVelocity(const pFloat3& vel) { mVelocity = vel; }
    inline void SetPosition(const pFloat3& pos) { mPosition = pos; }
    //inline void SetOrientation(const pQuat& orient) { mOrientation = orient; }

    inline void AddVelocity(const pFloat3& vel) { mVelocity += vel; }

    inline bool IsFixed() const { return mFixed; }
    inline void SetFixed(bool v) { mFixed = v; }

    inline pFloat GetMass() const { return mMass; }
    inline void SetMass(pFloat m) { mMass = m; }

    // Collision and colliders
    inline void SetCollider(PhysicsCollider* col) { mCollider = col; mCollider->GetRef()->AddReference(); }
    inline PhysicsCollider* GetCollider() const { return mCollider; }

    CollisionData Collide(const PhysicsObject& other) const;

    // Integration
    void Integrate(float dt);
};
    