#include "PhysicsObject.h"

#include "PhysicsObjectSphere.h"
#include "PhysicsObjectPlane.h"
#include "PhysicsObjectAABB.h"

void PhysicsObject::Integrate(float dt) {
    mPosition += mVelocity * dt;
}

CollisionData PhysicsObject::Collide(const PhysicsObject& other) const {
    if( mCollider->GetShapeType() == PhysicsShapeType::Sphere && other.mCollider->GetShapeType() == PhysicsShapeType::Sphere ) {
        // Both spheres
        PhysicsObjectSphere* Self = (PhysicsObjectSphere*)this;
        PhysicsObjectSphere* Other = (PhysicsObjectSphere*)&other;

        pFloat rDist = Self->GetRadius() + Other->GetRadius();
        pFloat cDist = ((pFloat3)other.GetPosition() - GetPosition()).length();

        // Check for intersection
        return { (cDist < rDist), cDist - rDist };
    } else if( mCollider->GetShapeType() == PhysicsShapeType::AABB && other.mCollider->GetShapeType() == PhysicsShapeType::AABB ) {
        // Both AABBs
        PhysicsObjectAABB* Self = (PhysicsObjectAABB*)this;
        PhysicsObjectAABB* Other = (PhysicsObjectAABB*)&other;

        pFloat dist = ((pFloat3)Other->GetMinPos() - Self->GetMaxPos()).max(
            (pFloat3)Self->GetMinPos() - (pFloat3)Other->GetMaxPos()).max();

        // Check for intersection
        return { (dist < 0), dist };
    } else if( mCollider->GetShapeType() == PhysicsShapeType::Plane && other.mCollider->GetShapeType() == PhysicsShapeType::Sphere ) {
        // Both AABBs
        PhysicsObjectPlane* Self = (PhysicsObjectPlane*)this;
        PhysicsObjectSphere* Other = (PhysicsObjectSphere*)&other;

        float dFsc = Self->GetDistance() + Self->GetNormal().dot(other.GetPosition()); // Distance from sphere center
        float dFs = dFsc - Other->GetRadius(); // Distance from sphere

        return { dFs < 0, dFs };
    }

    return {};
}
