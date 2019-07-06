#include "PhysicsEngine.h"

void PhysicsEngine::PushObject(PhysicsObject* obj) {
    objects.push_back(obj);
}

void PhysicsEngine::PopObject(PhysicsObject* obj) {
    auto iter = std::find(objects.begin(), objects.end(), obj);

    if( iter == objects.end() ) {
        return; // Not found
    } else {
        objects.erase(iter); // 
    }
}

void PhysicsEngine::Dispatch(float dt) {
    // Default CPU Dispatch function
    for( auto obj : objects ) {
        obj->Integrate(dt);

        if( !obj->IsFixed() ) {
            obj->AddVelocity(obj->GetVelocity() * mFriction * dt * -1);
            obj->AddAcceleration(mGravity * dt);
        }
    }

    // Handle collisions
    for( int i = 0; i < GetNumObjects(); i++ ) {
        for( int j = i + 1; j < GetNumObjects(); j++ ) {
            CollisionData data = objects[i]->Collide(*objects[j]);

            if( data.GetResult() ) {
                // Bounce
                pFloat3 dir1 = data.GetNormal();
                dir1 = dir1.normalized();

                pFloat3 tmp1 = objects[i]->GetVelocity().normalized();
                pFloat3 dir2 = dir1.Reflected(tmp1);

                pFloat3 r1 = objects[i]->GetVelocity().Reflected(dir2); // Velocity 1
                pFloat3 r2 = objects[j]->GetVelocity().Reflected(dir1); // Velocity 2

                objects[i]->SetAcceleration(0);
                objects[j]->SetAcceleration(0);

                objects[i]->SetVelocity(r1);
                objects[j]->SetVelocity(r2);
            }
        }
    }
}

void PhysicsEngine::Release() {
    for( auto v = objects.begin(); v != objects.end(); ++v ) {
        v = objects.erase(v);
    }
}
