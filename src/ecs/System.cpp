#include <ecs/System.h>

namespace ecs{
BaseSystem::Family BaseSystem::_family_counter;
BaseSystem::~BaseSystem(){
}

void SystemManager::updateAll(TimeDelta t){
    assert(_initialized&&"SystemManager is not initialized");
    for(auto& [first,second]:_systems){
        second->update(_entity_manager,_event_manager,t);
    }
}

void SystemManager::configure(){
    for(auto& pair:_systems){
        pair.second->configure(_entity_manager,_event_manager);
    }
    _initialized=true;
}

void BaseSystem::configure(EventManager& events){

}

} // namespace ecs