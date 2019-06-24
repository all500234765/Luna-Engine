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
        pFloat3 Normal = (other.GetPosition() - GetPosition());
        pFloat cDist = Normal.length();
        Normal /= cDist;

        pFloat dist = cDist - rDist;

        // Check for intersection
        return { dist < 0, Normal * dist };

        // 
    } else if( mCollider->GetShapeType() == PhysicsShapeType::AABB && other.mCollider->GetShapeType() == PhysicsShapeType::AABB ) {
        // Both AABBs
        PhysicsObjectAABB* Self = (PhysicsObjectAABB*)this;
        PhysicsObjectAABB* Other = (PhysicsObjectAABB*)&other;

        pFloat3 Normal = (Other->GetMinPos() - Self->GetMaxPos()).max(Self->GetMinPos() - Other->GetMaxPos());

        // Check for intersection
        return { (Normal.max() < 0), Normal };

        // 
    } else if( mCollider->GetShapeType() == PhysicsShapeType::Plane && other.mCollider->GetShapeType() == PhysicsShapeType::Sphere ) {
        // Plane vs Sphere
        PhysicsObjectPlane* Self = (PhysicsObjectPlane*)this;
        PhysicsObjectSphere* Other = (PhysicsObjectSphere*)&other;

        float dFsc = Self->GetDistance() + Self->GetNormal().dot(other.GetPosition()); // Distance from sphere center
        float dFs = dFsc - Other->GetRadius(); // Distance from sphere

        return { dFs < 0, Self->GetNormal() * dFs };

        // 
    } else if( mCollider->GetShapeType() == PhysicsShapeType::Sphere && other.mCollider->GetShapeType() == PhysicsShapeType::Plane ) {
        // Sphere vs Plane
        PhysicsObjectSphere* Self  = (PhysicsObjectSphere*)this;
        PhysicsObjectPlane* Other = (PhysicsObjectPlane*)&other;

        float dFsc = Other->GetDistance() + Other->GetNormal().dot(this->GetPosition()); // Distance from sphere center
        float dFs = dFsc - Self->GetRadius(); // Distance from sphere

        return { dFs < 0, Other->GetNormal() * dFs };

        // 
    }

    return {};
}
