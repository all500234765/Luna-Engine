#include "PhysicsObjectPlane.h"

PhysicsObjectPlane::PhysicsObjectPlane(PhysicsCollider* col) {
    SetCollider(col);
}

// TODO
/*const PhysicsObjectPlane& PhysicsObjectPlane::Normalized() const {
    pFloat len = mNormal.length();
    return PhysicsObjectPlane(mNormal / len, mDistance / len);
}*/

