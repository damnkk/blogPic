#include <ecs/Entity.h>

namespace ecs{
    const Entity::Id Entity::INVALID_ID;
    ComponentTypeCounterBase::TypeID ComponentTypeCounterBase::_typeId = 0;

    void Entity::invalidate(){
        _id = INVALID_ID;
        _em = nullptr;
    }

    void Entity::destroy(){
        assert(valid());
        _em->destroy(this->id());
        invalidate();
    }

    std::bitset<MAX_COMPONENTS> Entity::component_mask() const{
        return _em->componentMask(_id);
    }

    EntityManager::~EntityManager(){
        reset();
    }

    void EntityManager::reset(){
        for(BasePool* pool:_componentPools){
            if(pool) delete pool;
        }
        for(BaseComponentHelper* helper :_componentHelpers){
            if(helper) delete  helper;
        }

        _componentHelpers.clear();
        _componentPools.clear();
        _entityComponentMasks.clear();
        _freeList.clear();
        _entityVersions.clear();
        Index_counter = 0;
    }

    // EntityCreatedEvent::~EntityCreatedEvent() {}
    // EntityDestroyedEvent::~EntityDestroyedEvent() {}


} // namespace ecs