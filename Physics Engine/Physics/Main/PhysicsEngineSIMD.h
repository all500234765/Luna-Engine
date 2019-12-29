#pragma once

#include "pc.h"
#include "PhysicsEngine.h"

class PhysicsEngineSIMD: PhysicsEngine {
private:

public:
    virtual void Dispatch(float dt) override;

};
