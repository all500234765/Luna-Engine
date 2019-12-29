#pragma once

#include "pc.h"
#include "Engine/Utility/ReferenceCounter.h"
#include "../PhysicsTypes.h"
#include "Physics/Collision/CollisionData.h"

class PhysicsCollider {
private:
    PhysicsShapeType mType = PhysicsShapeType::Undefined;
    ReferenceCounter* ref = 0;

public:
    PhysicsCollider(): ref(new ReferenceCounter()) {};
    PhysicsCollider(PhysicsShapeType type): mType(type), ref(new ReferenceCounter()) {};

    // Shape
    inline PhysicsShapeType GetShapeType() const { return mType; }
    inline void SetType(PhysicsShapeType type) { mType = type; }

    inline ReferenceCounter* GetRef() { return ref; }
};
