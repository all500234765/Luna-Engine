#include "pc.h"
#include "ECSComponent.hpp"

std::vector<std::tuple< ECSComponentCreateFunc, ECSComponentFreeFunc, size_t >>* BaseECSComponent::ComponentTypes(0);

uint32 BaseECSComponent::_RegisterComponentType(ECSComponentCreateFunc createfn, ECSComponentFreeFunc freefn, size_t size) {
    if( ComponentTypes == nullptr ) {
        ComponentTypes = new std::vector<std::tuple< ECSComponentCreateFunc, ECSComponentFreeFunc, size_t >>();
    }

    uint32 _ComponentID = (uint32)ComponentTypes->size();
    ComponentTypes->push_back({createfn, freefn, size});
    return _ComponentID;
}
