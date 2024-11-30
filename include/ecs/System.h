#ifndef SYSTEM_H
#define SYSTEM_H
#include <cstdint>
#include <unordered_map>
#include <utility>
#include <cassert>
#include <ecs/Entity.h>
#include <ecs/Event.h>
#include <cinder/Cinder.h>
#include <cinder/Timer.h>

namespace ecs{ 
    typedef float TimeDelta;
class SystemManager;

class BaseSystem:public ci::Noncopyable{
public:
    typedef size_t Family;
    virtual ~BaseSystem();

    virtual void configure(EntityManager& entities, EventManager& events){
        configure(events);
    }

    virtual void configure(EventManager& events);
    virtual void update (EntityManager& entities,EventManager& events,double dt){};

    static Family _family_counter;
protected:
};

template<typename Derived>
class System:public BaseSystem{
public:
    virtual ~System(){}
private:
    friend class SystemManager;
    static Family family(){
        static Family id = BaseSystem::_family_counter++;
        return id;
    }
};

class SystemManager: ci::Noncopyable{
public:
    SystemManager(EntityManager& entity_manager,
                  EventManager& event_manager):_entity_manager(entity_manager),_event_manager(event_manager){}

template<typename S>
void add(std::shared_ptr<S> system){
    _systems.insert(std::make_pair(S::family(),system));
}

template<typename S,typename ...Args>
std::shared_ptr<S> add(Args&& ... args){
    std::shared_ptr<S> system = std::make_shared<S>(std::forward<Args>(args)...);
    add(system);
    return system;
}

template<typename S>
std::shared_ptr<S> system(){
    auto it = _systems.find(S::family());
    return std::shared_ptr<S>(std::static_pointer_cast<S>(it->second));
}

template<typename S>
void update (TimeDelta t){
    assert(_initialized&&"SystemManager::configure() not called");
    std::shared_ptr<S> s = system<S>();
    if(s){
        s->update(_entity_manager,_event_manager,t);
    }
}

void updateAll(TimeDelta t);

void configure();


private:
    bool _initialized = false;
    EntityManager& _entity_manager;
    EventManager& _event_manager;
    std::unordered_map<BaseSystem::Family,std::shared_ptr<BaseSystem>> _systems;
};


}// namespace ecs


#endif // SYSTEM_H
