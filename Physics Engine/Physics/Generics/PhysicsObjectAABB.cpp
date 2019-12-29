#include "pc.h"
#include "PhysicsObjectAABB.h"

PhysicsObjectAABB::PhysicsObjectAABB(PhysicsCollider* col) {
    SetCollider(col);
}
