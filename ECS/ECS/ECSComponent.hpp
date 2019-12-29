#pragma once

#include "pc.h"

struct BaseECSComponent;

typedef unsigned int uint32;
typedef unsigned char uint8;
typedef void* EntityHandle;

typedef size_t(*ECSComponentCreateFunc)(std::vector<uint8>& memory, EntityHandle entity, BaseECSComponent* comp);
typedef void(*ECSComponentFreeFunc)(BaseECSComponent* comp);

#define NULL_HANDLE nullptr

struct BaseECSComponent {
public:
    static uint32 _RegisterComponentType(ECSComponentCreateFunc createfn, ECSComponentFreeFunc freefn, size_t size);
    EntityHandle _Entity = NULL_HANDLE;

    inline static ECSComponentCreateFunc GetTypeCreate(uint32 id) { return std::get<0>((*ComponentTypes)[id]); }
    inline static ECSComponentFreeFunc   GetTypeFree  (uint32 id) { return std::get<1>((*ComponentTypes)[id]); }
    inline static size_t                 GetTypeSize  (uint32 id) { return std::get<2>((*ComponentTypes)[id]); }

    inline static bool IsValid(uint32 id) {
        return id < ComponentTypes->size();
    }
private:
    static std::vector<std::tuple< ECSComponentCreateFunc, ECSComponentFreeFunc, size_t >>* ComponentTypes;
};

template<typename T>
struct ECSComponent: public BaseECSComponent {
    static const ECSComponentCreateFunc _CREATE;
    static const ECSComponentFreeFunc   _FREE;
    static const uint32 _ID;
    static const size_t _SIZE;
};

template<typename Component>
size_t ECSComponentCreate(std::vector<uint8>& memory, EntityHandle entity, BaseECSComponent* comp) {
    size_t index = memory.size();
    memory.resize(index + Component::_SIZE);
    Component* component = new(&memory[index]) Component(*(Component*)comp);
    component->_Entity = entity;
    return index;
}

template<typename Component>
void ECSComponentFree(BaseECSComponent* comp) {
    Component* component = (Component*)comp;
    component->~Component();
}

template<typename T>
const uint32 ECSComponent<T>::_ID(BaseECSComponent::_RegisterComponentType(ECSComponentCreate<T>, ECSComponentFree<T>, sizeof(T)));

template<typename T>
const size_t ECSComponent<T>::_SIZE(sizeof(T));

template<typename T>
const ECSComponentCreateFunc ECSComponent<T>::_CREATE(ECSComponentCreate<T>);

template<typename T>
const ECSComponentFreeFunc ECSComponent<T>::_FREE(ECSComponentFree<T>);
