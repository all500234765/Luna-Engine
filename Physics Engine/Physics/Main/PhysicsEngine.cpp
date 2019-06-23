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
}
