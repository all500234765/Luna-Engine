#pragma once

#include "PhysicsEngine.h"

class PhysicsEngineSIMD: PhysicsEngine {
private:

public:
    virtual void Dispatch(float dt) override;

};
