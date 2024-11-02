#ifndef EVENT_H
#define EVENT_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <list>
#include <unordered_map>
#include <memory>
#include <utility>
#include <cinder/Cinder.h>
#include <cinder/Signals.h>
#include <cinder/app/Event.h>
#include <iostream>

namespace ecs{
    struct EventTypeIDBase{
    public:
        typedef std::size_t TypeID;
    protected:
        static TypeID TypeCount;
    };

    template<typename Derived>
    struct EventTypeID: public EventTypeIDBase{
        static const TypeID getID(){
            static TypeID id = TypeCount++;
            return id;
        }
    };

    struct BaseEvent{
        virtual ~BaseEvent() = default;

    }; 
 
    typedef cinder::signals::Signal<void(void*)> EventSignal;
    typedef std::shared_ptr<EventSignal> EventSignalSPtr;

    struct BaseReceiver{
        BaseReceiver(){};
        virtual ~BaseReceiver(){
            for(auto& conn :_connections){
                if(conn.second.isConnected()){
                    conn.second.disconnect();
                }
            }
        }

        std::size_t getConnectionCount() const {
            size_t res = 0;
            for(auto& conn:_connections){
                if(conn.second.isConnected()) ++res;
            }
            return res;
        }
    private:
        friend class EventManager;
        std::map<EventTypeIDBase::TypeID, cinder::signals::Connection> _connections;
    };

    template<typename Derived>
    struct Receiver:public BaseReceiver{
        Receiver(){}
        virtual ~Receiver(){}
        virtual void receive(BaseEvent& event){
        }
    };

    class EventManager:public cinder::Noncopyable{
    public:
        EventManager(){}
        ~EventManager(){}

        template<typename E, typename Receiver>
        void subscribe(Receiver& receiver){
            auto signal = signal_for(EventTypeID<E>::getID());
            auto conn = signal->connect([&receiver](void* event){
                E* e = static_cast<E*>(event);
                receiver.receive(*e);
            });
            BaseReceiver& baseRcv = receiver;
            baseRcv._connections[EventTypeID<E>::getID()] =  conn;
        }

        template<typename E, typename Receiver>
        void unsubscribe(Receiver& receiver){
            auto find = receiver._connections.find(EventTypeID<E>::getID());
            if(find!=receiver._connections.end()){
                find->second.disconnect();
                receiver._connections.erase(find);
            }
        }
        template<typename E, typename ... Args>
        void emit(Args&& ... args){
            E event = E(std::forward<Args>(args)...);
            auto signal = signal_for(EventTypeID<E>::getID());
            if(signal){
                signal->emit(&event);
            }
        }
    protected:
        EventSignalSPtr signal_for(size_t id){
            if(id>=_handles.size()){
                _handles.resize(id+1);
            }
            if(!_handles[id]){
                _handles[id] = std::make_shared<EventSignal>();
            }
            return _handles[id];
        }
        std::vector<EventSignalSPtr> _handles;

    };

} // namespace ecs


#endif // EVENT_H