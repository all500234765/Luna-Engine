#include "ECSSystem.hpp"

bool BaseECSSystem::IsValid() {
    for( uint32 i = 0; i < ComponentFlags.size(); i++ ) {
        if( (ComponentFlags[i] & BaseECSSystem::Flags::Optional) == 0 ) {
            return true;
        }
    }

    return false;
}

bool ECSSystemList::RemoveSystem(BaseECSSystem* Sys) {
    for( uint32 i = 0; i < Systems.size(); i++ ) {
        if( Sys == Systems[i] ) {
            // Not doing swap
            Systems.erase(Systems.begin() + i);
            return true;
        }
    }

    return false;
}

bool ECSSystemList::HasSystem(BaseECSSystem* Sys) const {
    for( uint32 i = 0; i < Systems.size(); i++ ) {
        if( Sys == Systems[i] ) { return true; }
    }

    return false;
}

void ECSSystemList::Clear() {
    if( !Systems.size() ) return;
    for( uint32 i = 0; i < Systems.size(); i++ ) {
        Systems.erase(Systems.begin() + i);
    }
}
