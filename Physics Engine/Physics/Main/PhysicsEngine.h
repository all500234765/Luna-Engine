#pragma once

#include "pc.h"
#include "..\Generics\PhysicsObject.h"

/// DX11 Build: 
/// SIMD Build: 
/// 

// TODO: GPU, SIMD and separated thread 
// API "independent"
class PhysicsEngine {
private:
    std::vector<PhysicsObject*> objects;
    pFloat3 mGravity;
    pFloat3 mAirResistance;
    pFloat  mFriction;

public:
    void PushObject(PhysicsObject* obj);
    void PopObject(PhysicsObject* obj);

    virtual void Dispatch(float dt);
    virtual void Release();

    inline const PhysicsObject* GetObjectP(unsigned int index) const { return objects[index]; }
    inline size_t GetNumObjects() const { return objects.size(); }

    inline const pFloat3& GetGravity() const { return mGravity; }
    inline void SetGravity(const pFloat3& g) { mGravity = g; }

    inline const pFloat3& GetAirResistance() const { return mAirResistance; }
    inline void SetAirResistance(const pFloat3& r) { mAirResistance = r; }

    inline pFloat GetFriction() const { return mFriction; }
    inline void SetFriction(pFloat f) { mFriction = f; }
};
