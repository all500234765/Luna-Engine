#include "pc.h"
#include "ECS.hpp"

void ECS::DeleteComponent(uint32 compID, uint32 index) {
    std::vector<uint8>& arr = Components[compID];

    size_t TypeSize = BaseECSComponent::GetTypeSize(compID);
    ECSComponentFreeFunc freefn = BaseECSComponent::GetTypeFree(compID);

    size_t srcIndex = arr.size() - TypeSize;
    BaseECSComponent* srcComp = (BaseECSComponent*)&arr[srcIndex];
    BaseECSComponent* destComp = (BaseECSComponent*)&arr[index];
    
    freefn(destComp);
    
    // Remove final element of an vector
    if( index == srcIndex ) {
        arr.resize(srcIndex);
        return;
    }

    // Update entities
    memcpy(destComp, srcComp, TypeSize);

    std::vector<std::pair<uint32, uint32>>& srcComps = GetEntityHandle(srcComp->_Entity);
    for( uint32 i = 0; i < srcComps.size(); i++ ) {
        if( compID == srcComps[i].first && srcIndex == srcComps[i].second ) {
            srcComps[i].second = index;
            break;
        }
    }

    arr.resize(srcIndex);
}

bool ECS::RemoveComponentInternal(EntityHandle handle, uint32 compID) {
    std::vector<std::pair<uint32, uint32>>& eComps = GetEntityHandle(handle);
    for( size_t i = 0; i < eComps.size(); i++ ) {
        if( compID == eComps[i].first ) {
            // Delete component
            DeleteComponent(eComps[i].first, eComps[i].second);
            
            // Remove it from vector
            size_t srcIndex = eComps.size() - 1;
            size_t destIndex = i;

            eComps[destIndex] = eComps[srcIndex];
            eComps.pop_back();

            return true;
        }
    }

    return false;
}

void ECS::AddComponentInternal(std::vector<std::pair<uint32, uint32>>& entity, uint32 compID, BaseECSComponent* comp, EntityHandle handle) {
    ECSComponentCreateFunc createfn = BaseECSComponent::GetTypeCreate(compID);
    std::pair<uint32, uint32> Pair;
    Pair.first = compID;
    Pair.second = createfn(Components[compID], handle, comp);
    entity.push_back(Pair);
}

BaseECSComponent* ECS::GetComponentInternal(std::vector<std::pair<uint32, uint32>>& eComps, std::vector<uint8>& arr, uint32 compID) {
    for( uint32 i = 0; i < eComps.size(); i++ ) {
        if( compID == eComps[i].first ) {
            return (BaseECSComponent*)&arr[eComps[i].second];
        }
    }

    return nullptr;
}

void ECS::UpdateMultiComponentSystem(float dt, uint32 index, const std::vector<uint32>& compTypes, 
                                     std::vector<BaseECSComponent*>& compParam, std::vector<std::vector<uint8>*>& compArrays, 
                                     ECSSystemList& Systems) {
    const std::vector<uint32>& compFlags = Systems[index]->GetComponentFlags();

    compParam.resize(std::max(compParam.size(), compTypes.size()));
    compArrays.resize(std::max(compArrays.size(), compTypes.size()));

    // LUT
    for( uint32 i = 0; i < compTypes.size(); i++ ) {
        compArrays[i] = &Components[compTypes[i]];
    }

    // 
    uint32 minSzIndex = FindLCComponent(compTypes, compFlags);
    size_t TypeSize = BaseECSComponent::GetTypeSize(compTypes[minSzIndex]);
    std::vector<uint8>& arr = *compArrays[minSzIndex];

    // 
    for( uint32 i = 0; i < arr.size(); i += TypeSize ) {
        compParam[minSzIndex] = (BaseECSComponent*)&arr[i];
        std::vector<std::pair<uint32, uint32>>& eComp = GetEntityHandle(compParam[minSzIndex]->_Entity);

        bool bIsValid = true;
        for( uint32 j = 0; j < compTypes.size(); j++ ) {
            if( j == minSzIndex ) {
                // If entity has this component -> entity is invalid for this system
                if( compFlags[j] & BaseECSSystem::Flags::Exclude ) { bIsValid = false; }
                continue;
            }

            compParam[j] = GetComponentInternal(eComp, *compArrays[j], compTypes[j]);
            if( compParam[j] == NULL_HANDLE ) {
                // Must have component // We do care if this entity doesn't have this component
                if( (compFlags[j] & BaseECSSystem::Flags::Optional) == 0
                 && (compFlags[j] & BaseECSSystem::Flags::Exclude ) == 0) {
                    bIsValid = false;
                    break;
                }
            } else {
                // If entity has this component -> entity is invalid for this system
                if( compFlags[j] & BaseECSSystem::Flags::Exclude ) {
                    bIsValid = false;
                    break;
                }
            }
        }

        if( bIsValid ) {
            Systems[index]->UpdateComponents(dt, &compParam[0]);
        }
    }
}

uint32 ECS::FindLCComponent(const std::vector<uint32>& compTypes, const std::vector<uint32>& compFlags) {
    uint32 minSize = (uint32)-1; // Components[compTypes[0]].size() / BaseECSComponent::GetTypeSize(compTypes[0]);
    uint32 minIndex = (uint32)-1;

    for( uint32 i = 0; i < compTypes.size(); i++ ) {
        if( (compFlags[i] & BaseECSSystem::Flags::Optional) == BaseECSSystem::Flags::Optional ) {
            continue;
        }

        size_t TypeSize = BaseECSComponent::GetTypeSize(compTypes[i]);
        uint32 size = Components[compTypes[i]].size() / TypeSize;
        
        if( size <= minSize ) {
            minSize = size;
            minIndex = i;
        }
    }

    return minIndex;
}

ECS::~ECS() {
    for( std::map< uint32, std::vector<uint8> >::iterator it = Components.begin(); it != Components.end(); ++it ) {
        size_t TypeSize = BaseECSComponent::GetTypeSize(it->first);
        ECSComponentFreeFunc freefn = BaseECSComponent::GetTypeFree(it->first);

        for( uint32 i = 0; i < it->second.size(); i += TypeSize ) {
            freefn((BaseECSComponent*)&it->second[i]);
        }
    }

    for( uint32 i = 0; i < Entities.size(); i++ ) {
        delete Entities[i];
    }
}

EntityHandle ECS::MakeEntity(BaseECSComponent** eComps, const uint32* compIDs, size_t numComp) {
    std::pair< uint32, std::vector<std::pair<uint32, uint32>> >* NewEntity = new std::pair< uint32, std::vector<std::pair<uint32, uint32>> >();
    EntityHandle handle = (EntityHandle)NewEntity;

    for( uint32 i = 0; i < numComp; i++ ) {
        // Check if CompID is valid
        if( !BaseECSComponent::IsValid(compIDs[i]) ) {
            // TODO: Log
            // [ECS]: Error: compIDs[i] is invalid component type.

            // 
            delete NewEntity;
            return NULL_HANDLE;
        }

        AddComponentInternal(NewEntity->second, compIDs[i], eComps[i], handle);
    }

    NewEntity->first = Entities.size();
    Entities.push_back(NewEntity);

    return handle;
}

void ECS::RemoveEntity(EntityHandle handle) {
    std::vector< std::pair<uint32, uint32> >& entity = GetEntityHandle(handle);
    for( uint32 i = 0; i < entity.size(); i++ ) {
        DeleteComponent(entity[i].first, entity[i].second);
    }

    uint32 destIndex = GetEntityIndex(handle);
    uint32 srcIndex  = Entities.size() - 1;

    // Delete entity and replace with last entity
    delete Entities[destIndex];
    Entities[destIndex] = Entities[srcIndex];
    Entities[destIndex]->first = destIndex;
    Entities.pop_back();
}

void ECS::UpdateSystems(ECSSystemList& Systems, float dt) {
    std::vector<BaseECSComponent*> compParam;
    std::vector<std::vector<uint8>*> compArrays;

    for( uint32 i = 0; i < Systems.size(); i++ ) {
        const std::vector<uint32>& compTypes = Systems[i]->GetComponentTypes();

        if( compTypes.size() == 1 ) {
            // Simple case
            size_t TypeSize = BaseECSComponent::GetTypeSize(compTypes[0]);
            std::vector<uint8>& arr = Components[compTypes[0]];

            for( uint32 j = 0; j < arr.size(); j += TypeSize ) {
                BaseECSComponent* comp = (BaseECSComponent*)&arr[j];
                Systems[i]->UpdateComponents(dt, &comp);
            }
        } else {
            // 
            UpdateMultiComponentSystem(dt, i, compTypes, compParam, compArrays, Systems);
        }
    }
}
