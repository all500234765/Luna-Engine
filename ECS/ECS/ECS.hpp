#pragma once

#include "ECSComponent.hpp"
#include "ECSSystem.hpp"
#include <map>
#include <vector>
#include <memory>
#include <algorithm>

class ECS {
private:
    std::map< uint32, std::vector<uint8> > Components;
    std::vector< std::pair< uint32, std::vector<std::pair<uint32, uint32>> >* > Entities;

    inline std::pair< uint32, std::vector<std::pair<uint32, uint32>> >* GetRawTypeHandle(EntityHandle handle) {
        return (std::pair< uint32, std::vector<std::pair<uint32, uint32>> >*)handle;
    }

    inline uint32 GetEntityIndex(EntityHandle handle) {
        return GetRawTypeHandle(handle)->first;
    }

    inline std::vector<std::pair<uint32, uint32>>& GetEntityHandle(EntityHandle handle) {
        return GetRawTypeHandle(handle)->second;
    }

    void DeleteComponent(uint32 compID, uint32 index);
    bool RemoveComponentInternal(EntityHandle handle, uint32 compID);
    void AddComponentInternal(std::vector<std::pair<uint32, uint32>>& entity, uint32 compID, BaseECSComponent* comp, EntityHandle handle);
    BaseECSComponent* GetComponentInternal(std::vector<std::pair<uint32, uint32>>& eComps, 
                                           std::vector<uint8>& arr, uint32 compID);

    void UpdateMultiComponentSystem(float dt, uint32 index, const std::vector<uint32>& compTypes,
                                    std::vector<BaseECSComponent*>& compParam, 
                                    std::vector<std::vector<uint8>*>& compArrays, ECSSystemList& Systems);
    uint32 FindLCComponent(const std::vector<uint32>& compTypes, const std::vector<uint32>& compFlags);
public:
    ECS() {};
    ECS(const ECS& ecs) { (void)ecs; };
    void operator=(const ECS& ecs) { (void)ecs; }
    ~ECS();

    // Entity
    EntityHandle MakeEntity(BaseECSComponent** comps, const uint32* compIDs, size_t numComp);
    void RemoveEntity(EntityHandle handle);

    template<class A, class B, class C, class D, class E, class F, class G, class K, class L, class M>
    EntityHandle MakeEntity(A& c1, B& c2, C& c3, D& c4, E& c5, F& c6, G& c7, K& c8, L& c9, M& c10) {
        BaseECSComponent* comps[] = { &c1, &c2, &c3, &c4, &c5, &c6, &c7, &c8, &c9, &c10 };
        uint32 compIDs[] = { A::_ID, B::_ID, C::_ID, D::_ID, E::_ID, F::_ID, G::_ID, K::_ID, L::_ID, M::_ID };

        return MakeEntity(comps, compIDs, 10);
    }

    template<class A, class B, class C, class D, class E, class F, class G, class K, class L>
    EntityHandle MakeEntity(A& c1, B& c2, C& c3, D& c4, E& c5, F& c6, G& c7, K& c8, L& c9) {
        BaseECSComponent* comps[] = { &c1, &c2, &c3, &c4, &c5, &c6, &c7, &c8, &c9 };
        uint32 compIDs[] = { A::_ID, B::_ID, C::_ID, D::_ID, E::_ID, F::_ID, G::_ID, K::_ID, L::_ID };

        return MakeEntity(comps, compIDs, 9);
    }

    template<class A, class B, class C, class D, class E, class F, class G, class K>
    EntityHandle MakeEntity(A& c1, B& c2, C& c3, D& c4, E& c5, F& c6, G& c7, K& c8) {
        BaseECSComponent* comps[] = { &c1, &c2, &c3, &c4, &c5, &c6, &c7, &c8 };
        uint32 compIDs[] = { A::_ID, B::_ID, C::_ID, D::_ID, E::_ID, F::_ID, G::_ID, K::_ID };

        return MakeEntity(comps, compIDs, 8);
    }

    template<class A, class B, class C, class D, class E, class F, class G>
    EntityHandle MakeEntity(A& c1, B& c2, C& c3, D& c4, E& c5, F& c6, G& c7) {
        BaseECSComponent* comps[] = { &c1, &c2, &c3, &c4, &c5, &c6, &c7 };
        uint32 compIDs[] = { A::_ID, B::_ID, C::_ID, D::_ID, E::_ID, F::_ID, G::_ID };

        return MakeEntity(comps, compIDs, 7);
    }

    template<class A, class B, class C, class D, class E, class F>
    EntityHandle MakeEntity(A& c1, B& c2, C& c3, D& c4, E& c5, F& c6) {
        BaseECSComponent* comps[] = { &c1, &c2, &c3, &c4, &c5, &c6 };
        uint32 compIDs[] = { A::_ID, B::_ID, C::_ID, D::_ID, E::_ID, F::_ID };

        return MakeEntity(comps, compIDs, 6);
    }

    template<class A, class B, class C, class D, class E>
    EntityHandle MakeEntity(A& c1, B& c2, C& c3, D& c4, E& c5) {
        BaseECSComponent* comps[] = { &c1, &c2, &c3, &c4, &c5 };
        uint32 compIDs[] = { A::_ID, B::_ID, C::_ID, D::_ID, E::_ID };

        return MakeEntity(comps, compIDs, 5);
    }

    template<class A, class B, class C, class D>
    EntityHandle MakeEntity(A& c1, B& c2, C& c3, D& c4) {
        BaseECSComponent* comps[] = { &c1, &c2, &c3, &c4 };
        uint32 compIDs[] = { A::_ID, B::_ID, C::_ID, D::_ID };

        return MakeEntity(comps, compIDs, 4);
    }

    template<class A, class B, class C>
    EntityHandle MakeEntity(A& c1, B& c2, C& c3) {
        BaseECSComponent* comps[] = { &c1, &c2, &c3 };
        uint32 compIDs[] = { A::_ID, B::_ID, C::_ID };

        return MakeEntity(comps, compIDs, 3);
    }

    template<class A, class B>
    EntityHandle MakeEntity(A& c1, B& c2) {
        BaseECSComponent* comps[] = { &c1, &c2 };
        uint32 compIDs[] = { A::_ID, B::_ID };

        return MakeEntity(comps, compIDs, 2);
    }

    template<class A>
    EntityHandle MakeEntity(A& c1) {
        BaseECSComponent* comps[] = { &c1 };
        uint32 compIDs[] = { A::_ID };

        return MakeEntity(comps, compIDs, 1);
    }

    // Component
    template<typename Component>
    inline void AddComponent(EntityHandle entity, Component* comp) {
        AddComponentInternal(GetEntityHandle(entity), Component::_ID, comp, entity);
    };

    template<typename Component>
    Component* GetComponent(EntityHandle entity) {
        return (Component*)GetComponentInternal(GetEntityHandle(entity), Components[Component::_ID], Component::_ID);
    }

    template<typename Component>
    bool RemoveComponent(EntityHandle entity) {
        return RemoveComponentInternal(entity, Component::_ID);
    }

    // System
    void UpdateSystems(ECSSystemList& Systems, float dt);
};
