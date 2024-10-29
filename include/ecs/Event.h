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

class BaseEvent{
public:
    typedef std::size_t Family;
    virtual ~BaseEvent();
protected:
    static Family family_counter_;
};


struct QQEvent;

typedef cinder::signals::Signal<void(const QQEvent&)> EventSignal;
typedef std::shared_ptr<EventSignal> EventSignalPtr;

template<typename Derived>
class Event:public BaseEvent{
public:
    static Family family(){
        static Family family = family_counter_++;
        return family;
    }
};

class BaseReceiver{
    public:
    virtual ~BaseReceiver(){
        for(auto& [first,second]:_connections){
            second.disconnect();
        }
    }

    std::size_t getConnectCount() const {
        size_t count = 0;
        for(auto& [first,second]:_connections){
            if(second.isConnected()){
                count += 1;
            }
        }
        return count;
    }
    private:
    friend class MyEventManager;
    std::unordered_map<BaseEvent::Family, cinder::signals::Connection> _connections;
};


template<typename Derived>
class Receiver:public BaseReceiver{
    public:
    ~Receiver(){}
};

class MyEventManager{
public:
    MyEventManager(){}
    virtual ~MyEventManager(){}
    template<typename E,typename Receiver>
    void subscribe(Receiver& receiver){
        void (Receiver::*receive)(const E&)= &Receiver::receive;
        EventSignalPtr sigPtr = signal_for(Event<E>::family());
        auto connection = sigPtr->connect([&receiver](const E& event) {
            receiver.receive(event);
        });
        BaseReceiver* bReceiver = &receiver;
        receiver._connections[Event<E>::family()] = connection;
    }
 template<typename E,typename Receiver>
    void unsubscribe(Receiver& receiver){
        if(!_handles[Event<E>::family()]){
            return;
        }
        if(receiver._connections.find(Event<E>::family())!=receiver._connections.end()){
            receiver._connections[Event<E>::family()].disconnect();
            receiver._connections.erase(Event<E>::family());
        }
    }

    template<typename E>
    void emit(const Event<E>& event){
        auto sig = this->signal_for(Event<E>::family()); // 如果在类内部，确保使用 `this->`
        sig->emit(event);
    }

    template<typename E, typename ... Args>
    void emit(Args& ...args){
        auto sig = this->signal_for(Event<E>::family());
        sig->emit(std::forward<Args>(args)...);
    }

private:

EventSignalPtr signal_for(std::size_t id){
    if(_handles.size()<=id){
        _handles.resize(id+1);
    }
    if(!_handles[id]){
        _handles[id] = std::make_shared<EventSignal>();
    }
    return _handles[id];
}


template<typename T>
struct FunctionInvokeWrapper{
    explicit FunctionInvokeWrapper(std::function<void(const T&)> func):_func(func){
    }

    void operator()(const void* event){
        _func(*(static_cast<const T*>(event)));
    }

    std::function<void(const T&)> _func;
};

std::vector<EventSignalPtr> _handles;
};


#endif // EVENT_H