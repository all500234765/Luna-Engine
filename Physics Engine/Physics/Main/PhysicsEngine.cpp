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
    }

    // Handle collisions
    for( int i = 0; i < GetNumObjects(); i++ ) {
        for( int j = i + 1; j < GetNumObjects(); j++ ) {
            CollisionData data = objects[i]->Collide(*objects[j]);
            
            if( data.GetResult() ) {
                objects[i]->SetVelocity(objects[i]->GetVelocity() * -1);
                objects[j]->SetVelocity(objects[j]->GetVelocity() * -1);
            }
        }
    }
}

void PhysicsEngine::Release() {
    for( auto v = objects.begin(); v != objects.end(); v++ ) {
        v = objects.erase(v);
    }
}
