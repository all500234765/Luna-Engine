#pragma once

#include <vector>
#include "..\Generics\PhysicsObject.h"

/// DX11 Build: 
/// SIMD Build: 
/// 

// TODO: GPU, SIMD and separated thread 
// API "independent"
class PhysicsEngine {
private:
    std::vector<PhysicsObject*> objects;

public:
    void PushObject(PhysicsObject* obj);
    void PopObject(PhysicsObject* obj);

    virtual void Dispatch(float dt);

    inline const PhysicsObject* GetObjectP(unsigned int index) const { return objects[index]; }
    inline int GetNumObjects() const { return objects.size(); }
};
