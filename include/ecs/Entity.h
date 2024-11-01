#ifndef ECS_ENTITY_H
#define ECS_ENTITY_H
#include <iterator>
#include <ecs/Event.h>
#include <cinder/Cinder.h>
#include <ecs/Pool.h>
namespace ecs{
class EntityManager;
typedef std::uint64_t uint64_t;
typedef std::uint32_t uint32_t;
static const size_t MAX_COMPONENTS  = 64;

template<typename C,typename EM = EntityManager>
class ComponentHandle;

class Entity{

public:
    struct Id{
        Id():_id(0){};
        explicit Id(uint64_t id):_id(id){};
        Id(uint32_t index,uint32_t version):_id(uint64_t(index)|(uint64_t(version)<<32)){};
        bool operator==(const Id& other) const {
            return this->_id == other._id;
        }
        bool operator!=(const Id& other) const{
            return this->_id!= other._id;
        }
        bool operator<(const Id& other) const{
            return this->_id < other._id;
        }
        uint32_t index() const{
            return uint32_t(_id);
        }
        uint32_t version() const{
            return uint32_t(_id>>32);
        }
        private:
        friend class EntityManager;

        uint64_t _id;
    };
    static const Id INVALID_ID;
    Entity()=default;
    Entity(EntityManager* em,Id id):_em(em),_id(id){}
    Entity(const Entity& other)=default;
    Entity& operator=(const Entity& other) = default;

    operator bool() const{
        return valid();
    }

    bool operator == (const Entity& other) const{
        return this->_em == other._em && this->_id == other._id;
    }

    bool operator != (const Entity& other) const{
        return *this!=other;
    }

    bool operator<(const Entity& other) const{
        return other._id<this->_id;
    }
    bool valid() const;

    void invalidate();

    Id id() const{
        return _id;
    }

    template<typename C,typename ... Args>
    ComponentHandle<C> assign(Args&& ...args);
    template<typename C>
    ComponentHandle<C> assign_for_copy(const C& c);
    template<typename C,typename ... Args>
    ComponentHandle<C> replace(Args&& ...args);

    template<typename C>
    void remove();
    template<typename C,typename = typename std::enable_if<!std::is_const<C>::value>::type>
    ComponentHandle<C> component();
    template<typename C,typename = typename std::enable_if<std::is_const<C>::value>::type>
    const ComponentHandle<C,const EntityManager> component() const;

    template<typename ... Components>
    std::tuple<ComponentHandle<Components>...> components();
    template<typename ... Components>
    std::tuple<ComponentHandle<const Components>...> components() const;

    template<typename C>
    bool has_component() const;

    void destroy();
    std::bitset<MAX_COMPONENTS> component_mask() const;
    private:
    friend class EntityManager;
    Id _id=INVALID_ID;
    EntityManager* _em = nullptr;
};

template<typename C, typename EM>
class ComponentHandle{
public:
    typedef C ComponentType;
    ComponentHandle():_em(nullptr){}
    bool valid() const;
    operator bool() const ;
    ComponentType* operator->();
    const ComponentType* operator->() const;
    ComponentType& operator*();
    const ComponentType& operator*()const;
    ComponentType* get() ;
    const ComponentType* get() const;

    bool operator==(const ComponentHandle& other)const;
    bool operator!=(const ComponentHandle& other) const;

    void remove();
    Entity entity() ;
private:
    friend class EntityManager;
    ComponentHandle<C> (EM* em,Entity::Id id):_em(em),_id(id){};
    Entity::Id _id;
    EM* _em;
};

struct ComponentTypeCounterBase{
public:
    typedef size_t TypeID;
    void operator delete(void* p){
        fail();
    }
    void operator delete[](void* p){
        fail();
    }
protected:
    static void fail(){
        std::cerr<<"Cannot free component's memory directly, use Entity::destroy() instead"<<std::endl;
        abort();
    }
    static TypeID _typeId;
};

template<typename C>
class ComponentTypeCounter:public ComponentTypeCounterBase{
    public:
    static TypeID typeId(){
        static TypeID id = _typeId++;
        return id;
    }
};

struct EntityCreatedEvent:public BaseEvent{
    explicit EntityCreatedEvent(Entity entity):_entity(entity){}
    // ~EntityCreatedEvent() ;
    Entity _entity;
};

struct EntityDestroyedEvent:public BaseEvent{
    explicit EntityDestroyedEvent(Entity entity):_entity(entity){}
    // virtual ~EntityDestroyedEvent();

    Entity _entity;
};

template<typename C>
struct ComponentAddedEvent:public BaseEvent{
    explicit ComponentAddedEvent(Entity entity,ComponentHandle<C> component):_entity(entity),_component(component){};
    // virtual ~ComponentAddedEvent();
    Entity _entity;
    ComponentHandle<C> _component;
};

template<typename C>
struct ComponentRemovedEvent:public BaseEvent{
    explicit ComponentRemovedEvent(Entity entity, ComponentHandle<C> component):_entity(entity),_component(component){}
    // virtual ~ComponentRemovedEvent();
    Entity _entity;
    ComponentHandle<C> _component;
};

class BaseComponentHelper{
public:
    virtual ~BaseComponentHelper(){}
    virtual void remove_component(Entity e) = 0;
    virtual void copy_component_to(Entity src,Entity dst) = 0;
};

template<typename C>
class ComponentHelper:public BaseComponentHelper{
public:
    void remove_component(Entity e){
        e.remove<C>();
    }
    void copy_component_to(Entity src,Entity dst){
        dst.assign_for_copy(*(src.component<C>().get()));
    }
};  

class EntityManager:public cinder::Noncopyable{
public:
    typedef std::bitset<MAX_COMPONENTS> ComponentMask;
    EntityManager(EventManager& eventManager):_eventManager(eventManager){}
    virtual ~EntityManager();
    inline void assert_valid(Entity::Id id) const{
        assert(_entityComponentMasks.size()>id.index()&&"Entity::Id ID outside entity vector");
        assert(_entityVersions[id.index()]==id.version()&&"Entity::Id version mismatch");
    }

    size_t size() const{
        return _entityComponentMasks.size()-_freeList.size();
    }

    size_t capacity() const{
        return _entityComponentMasks.size();
    }

    bool valid(Entity::Id id) const{
        return id.index()<_entityComponentMasks.size()&&_entityVersions[id.index()]==id.version();
    }

    Entity create(){
        uint32_t index,version;
        if(_freeList.empty()){
            index = Index_counter ++;
            accomodateEntity(index);
            version = _entityVersions[index] = 1;
        }else{
            index = _freeList.back();
            _freeList.pop_back();
            version = _entityVersions[index];
        }
        Entity e(this,Entity::Id(index,version));
        EntityCreatedEvent event(e);
        _eventManager.emit<EntityCreatedEvent>(event);
        return e;
    }

    Entity create_from_copy(Entity original){
        assert(original.valid());
        auto clone = create();
        for(int i = 0;i<_componentHelpers.size();++i){
            auto helper = _componentHelpers[i];
            if(helper&&original.component_mask().test(i)){
                helper->copy_component_to(original,clone);
            }
        }
        return clone;
    }

    void destroy(Entity::Id entity){
        assert(valid(entity));
        uint32_t index= entity.index();
        auto componentMask = _entityComponentMasks[index];
        for(int i = 0;i<_componentHelpers.size();++i){
            auto helper = _componentHelpers[i];
            if(helper&&componentMask.test(i)){
                helper->remove_component(Entity(this,entity));
            }
        }
        EntityDestroyedEvent et(Entity(this,entity));
        _eventManager.emit<EntityDestroyedEvent>(et);
        _entityComponentMasks[index].reset();
        _entityVersions[index] +=1;
        _freeList.push_back(index);
    }

    Entity get(Entity::Id id){
        assert(valid(id));
        return Entity(this,id);
    }

    template<typename C,typename ...Args>
    ComponentHandle<C> assign(Entity::Id e, Args&&...args){
        assert_valid(e);
        ComponentTypeCounterBase::TypeID typeId = ComponentTypeCounter<C>::typeId();
        assert(!_entityComponentMasks[e.index()].test(typeId));
        Pool<C>* pool = accomodateComponent<C>();
        ::new(pool->get(e.index())) C(std::forward<Args>(args)...);
        _entityComponentMasks[e.index()].set(typeId);
        ComponentHandle<C> handle(this,e);
        _eventManager.emit<ComponentAddedEvent<C>>(Entity(this,e),handle);
        return handle;
    }

    template<typename C>
    void remove(Entity::Id e){
        assert_valid(e);
        ComponentTypeCounterBase::TypeID typeId = ComponentTypeCounter<C>::typeId();
        assert(_entityComponentMasks[e.index()].test(typeId));
        _entityComponentMasks[e.index()].reset(typeId);
        _componentPools[typeId]->destroy(e.index());
        _eventManager.emit<ComponentRemovedEvent<C>>(Entity(this,e),ComponentHandle<C>(this,e));
    }

    template<typename C>
    bool has_component(Entity::Id e){
        assert_valid(e);
        ComponentTypeCounterBase::TypeID typeId = ComponentTypeCounter<C>::typeId();
        if(typeId>=_componentPools.size()) return false;
        BasePool* pool = _componentPools[typeId];
        if(!pool||!_entityComponentMasks[e.index()].test(typeId)) return false;
        return true;
    }

    template<typename C, typename= typename std::enable_if<!std::is_const<C>::value>::type>
    ComponentHandle<C> component(Entity::Id e){
        assert_valid(e);
        ComponentTypeCounterBase::TypeID typeId = ComponentTypeCounter<C>::typeId();
        if(typeId>=_componentPools.size()){
            return ComponentHandle<C>();
        }
        Pool<C>* pool = static_cast<Pool<C>*>(_componentPools[typeId]);
        if(!pool||!_entityComponentMasks[e.index()].test(typeId)){
            return ComponentHandle<C>();
        }
        return ComponentHandle<C>(this,e);
    }

    template<typename C, typename = typename std::enable_if<std::is_const<C>::value>::type>
    const ComponentHandle<C,const EntityManager> component(Entity::Id e) const{
        assert_valid(e);
        ComponentTypeCounterBase::TypeID typeId = ComponentTypeCounter<C>::typeId();
        if(typeId>=_componentPools.size()){
            return ComponentHandle<C, const EntityManager>();
        }
        const Pool<C>* pool = static_cast<const Pool<C>*>(_componentPools[typeId]);
        if(!pool||!_entityComponentMasks[e.index()].test(typeId)){
            return ComponentHandle<C, const EntityManager>();
        }
        return ComponentHandle<C,const EntityManager>(this,e);
    }

    void reset();
private:
    template<typename C>
    Pool<C>* accomodateComponent(){
        ComponentTypeCounterBase::TypeID typeId = ComponentTypeCounter<C>::typeId();
        if(_componentPools.size()<=typeId){
            _componentPools.resize(typeId+1,nullptr);
        }
        if(!_componentPools[typeId]){
            Pool<C>* pool =new Pool<C>();
            pool->expand(Index_counter);
            _componentPools[typeId] = pool;
        }
        if(_componentHelpers.size()<=typeId){
            _componentPools.resize(typeId+1,nullptr);
        }
        if(!_componentHelpers[typeId]){
            _componentHelpers[typeId] = new ComponentHelper<C>();
        }
        return static_cast<Pool<C>*>(_componentPools[typeId]);
    }

    void accomodateEntity(uint32_t index){
        if(_entityComponentMasks.size()<=index){
            _entityComponentMasks.resize(index+1);
            _entityVersions.resize(index+1);
            for(auto& pool:_componentPools){
                if(pool)pool->expand(index+1);
            }
        }
    }

    template<typename C>
     C* getComponentPtr(Entity::Id e){
        assert_valid(e);
        ComponentTypeCounterBase::TypeID typeId = ComponentTypeCounter<C>::typeId();
        BasePool* pool = _componentPools[typeId];
        assert(pool);
        return static_cast<C*>(pool->get(e.index()));
    }

    template<typename C>
    const C* getComponentPtr(Entity::Id e) const {
        assert_valid(e);
        BasePool* pool = _componentPools[ComponentTypeCounter<C>::typeId()];
        assert(pool);
        return static_cast<const C*>(pool->get(e.index()));
    }

    ComponentMask componentMask(Entity::Id id) const{
        assert_valid(id);
        return _entityComponentMasks[id.index()];
    }

    template<typename C>
    ComponentMask componentMask(){
        ComponentMask mask;
        mask.set(ComponentTypeCounter<typename std::remove_const<C>::type>::typeId());
        return mask;
    }

    template<typename C1,typename C2, typename ...Components>
    ComponentMask componentMask(){
        ComponentMask mask;
        return componentMask<C1>()|componentMask<C2,Components...>();
    }
     
    friend class Entity;
    template<typename C,typename EM>
    friend class ComponentHandle;

    uint32_t Index_counter = 0;
    std::vector<BaseComponentHelper*> _componentHelpers;
    std::vector<ComponentMask> _entityComponentMasks;
    std::vector<BasePool*> _componentPools;
    std::vector<uint32_t> _entityVersions;
    std::vector<uint32_t> _freeList;
    EventManager& _eventManager;
};

template<typename C,typename ... Args>
ComponentHandle<C> Entity::assign(Args&& ...args){
    assert(valid());
    return _em->assign<C>(this->id(),std::forward<Args>(args)...);
}

template<typename C>
ComponentHandle<C> Entity::assign_for_copy(const C& c){
    assert(valid());
    return _em->assign<C>(this->id(),std::forward<const C&>(c));
}

template<typename C, typename ... Args>
ComponentHandle<C> Entity::replace(Args&& ...args){
    assert(valid());
    auto handle = component<C>();
    if(handle){
        *(handle.get()) = C(std::forward<Args>(args)...);
    }else{
        handle=  _em->assign<C>(this->id(),std::forward<Args>(args)...);
    }
    return handle;
}

template<typename C>
void Entity::remove(){
    assert(valid());
    _em->remove<C>(this->id());
}

template<typename C,typename>
ComponentHandle<C> Entity::component(){
    assert(valid());
    return _em->component<C>(this->id());
}

template<typename C,typename>
const ComponentHandle<C,const EntityManager> Entity::component() const{
    assert(valid());
    return const_cast<const EntityManager*>(_em)->component<const C>(this->id());
}

template<typename C>
bool Entity::has_component()const{
    assert(valid());
    return _em->has_component<C>(this->id());
}

inline bool Entity::valid() const{
    return _em&&_em->valid(_id);
}

inline std::ostream& operator<<(std::ostream& os, const Entity::Id& id){
    os<<"Entity::Id("<<id.index()<<"."<<id.version()<<")";
    return os;
}

inline std::ostream& operator<<(std::ostream& os,const Entity& e){
    os<<"Entity("<<e.id()<<")";
    return os;
}

template<typename C, typename EM>
inline ComponentHandle<C,EM>::operator bool()const{
    return this->_em&&this->_em->valid(this->_id)&&this->_em->template has_component<C>(this->_id);
}

template<typename C, typename EM>
inline bool ComponentHandle<C,EM>::valid() const{
    return this->_em&&this->_em->valid(this->_id)&&this->_em->template has_component<C>(this->_id);
}

template<typename C, typename EM>
inline C* ComponentHandle<C,EM>::operator->(){
    assert(valid());
    return this->_em->template getComponentPtr(this->_id);
}

template<typename  C, typename EM>
inline const C* ComponentHandle<C,EM>::operator->() const{
    assert(valid());
    return this->_em->template getComponentPtr<C>(this->_id);
}

template<typename C, typename EM>
inline C& ComponentHandle<C,EM>::operator*(){
    assert(valid());
    return *this->_em->template getComponentPtr<C>(this->_id);
}

template<typename C, typename EM>
inline const C& ComponentHandle<C,EM>::operator*()const{
    assert(valid());
    return *this->_em->template getComponentPtr<const C>(this->_id);
}

template<typename C, typename EM>
inline C* ComponentHandle<C,EM>::get() {
    assert(valid());
    return this->_em->template getComponentPtr<C>(this->_id);
}

template<typename C,typename EM>
inline const C* ComponentHandle<C,EM>::get() const{
    assert(valid());
    return this->_em->template getComponentPtr<const C>(this->_id);
}

template<typename C, typename EM>
inline void ComponentHandle<C,EM>::remove(){
    assert(valid());
    this->_em->template remove<C>(_id);
}

template<typename C,typename EM>
inline Entity ComponentHandle<C,EM>::entity(){
    assert(valid());
    return _em->get(_id);
}
} // namespace ecs

namespace std{
    template<> struct hash<ecs::Entity>{
        std::size_t operator()(const ecs::Entity& entity) const{
            return static_cast<std::size_t>(entity.id().index()^entity.id().version());
        }
    };

    template<> struct hash<const ecs::Entity>{
        std::size_t operator()(const ecs::Entity& entity) const{
            return static_cast<std::size_t>(entity.id().index()^entity.id().version());
        }
    };
}
#endif // ECS_ENTITY_H