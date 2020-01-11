#pragma once

#include "pc.h"

struct BaseECSComponent;

typedef unsigned int uint32;
typedef unsigned char uint8;
typedef void* EntityHandle;

typedef size_t(*ECSComponentCreateFunc)(std::vector<uint8>& memory, EntityHandle entity, BaseECSComponent* comp);
typedef void(*ECSComponentFreeFunc)(BaseECSComponent* comp);

#define NULL_HANDLE nullptr

struct ECSComponentSearializeHeader {
    uint32 _ID;
    uint32 _Align;
    size_t _SIZE;
};

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
    static const size_t _SERIALIZED_SIZE;

    virtual void Serialize(uint8_t* out) const;
    virtual std::vector<uint8_t>&& Serialize() const;

    virtual bool Deserialize(uint8_t* in);
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

template<typename Component>
void ECSComponent<Component>::Serialize(uint8_t* out) const {
    // Header
    ECSComponentSearializeHeader head{};
    head._ID    = _ID;
    head._SIZE  = _SIZE;
    head._Align = 0;
    memcpy(out, &head, sizeof(head));

    // Data
    memcpy((void*)(out + sizeof(head)), (Component*)this, sizeof(Component));
}

template<typename Component>
std::vector<uint8_t>&& ECSComponent<Component>::Serialize() const {
    std::vector<uint8_t> buff{};
    buff.reserve(sizeof(_SERIALIZED_SIZE));

    // Header
    ECSComponentSearializeHeader head{};
    head._ID    = _ID;
    head._SIZE  = _SIZE;
    head._Align = 0;
    memcpy((void*)buff.data(), &head, sizeof(head));

    // Data
    memcpy((void*)(buff.data() + sizeof(head)), (Component*)this, sizeof(Component));

    return std::move(buff);
}

template<typename Component>
inline bool ECSComponent<Component>::Deserialize(uint8_t* in) {
    // Header
    ECSComponentSearializeHeader head{};
    memcpy(&head, in, sizeof(head));

    if( head._ID != _ID || head._SIZE != _SIZE ) return false;

    // Data
    memcpy((Component*)this, (void*)(in + sizeof(head)), sizeof(Component));

    // Done
    return true;
}

template<typename T>
const uint32 ECSComponent<T>::_ID(BaseECSComponent::_RegisterComponentType(ECSComponentCreate<T>, ECSComponentFree<T>, sizeof(T)));

template<typename T>
const size_t ECSComponent<T>::_SIZE(sizeof(T));

template<typename T>
const size_t ECSComponent<T>::_SERIALIZED_SIZE(sizeof(T) - sizeof() + sizeof(ECSComponentSearializeHeader));

template<typename T>
const ECSComponentCreateFunc ECSComponent<T>::_CREATE(ECSComponentCreate<T>);

template<typename T>
const ECSComponentFreeFunc ECSComponent<T>::_FREE(ECSComponentFree<T>);
