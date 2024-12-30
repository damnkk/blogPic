#ifndef MYCOMPONENT_H
#define MYCOMPONENT_H
#include <cinder/gl/gl.h>
#include <rttr/rttr_enable.h>
#include <chrono>
#include <cinder/Json.h>
#include <MySerizalizable.h>
struct MyApp;
struct MyComponent : public MySerializable {
  virtual void preUpdate() {};
  virtual void update(double deltaTime) {};
  virtual void draw() {};
  virtual void postUpdate() {};
  virtual cinder::Json transfer() { return {}; };

  RTTR_ENABLE(MySerializable)
};

struct MyRenderable : public MyComponent {

  uint32_t _renderSortID = INT_MAX;
  RTTR_ENABLE(MyComponent)
};

struct ScreenRenderable : public MyComponent {
  cinder::gl::BatchRef                 batch;
  std::shared_ptr<cinder::CameraPersp> camera;
  RTTR_ENABLE(MyComponent)
};

class MyComponentPool {
 public:
  static MyComponentPool* instance() {
    static MyComponentPool pool;
    return &pool;
  }

  template<typename T>
  std::shared_ptr<T> createComponent(MyApp* app) {
    rttr::type type = rttr::type::get<T>();
    _componentPools[type.get_name().to_string()].push_back(std::make_shared<T>());
    return std::static_pointer_cast<T>(_componentPools[type.get_name().to_string()].back());
  }

  std::shared_ptr<MyComponent> getComponentFromUUID(const uuids::uuid uuid, std::string typeID) {
    for (auto& component : _componentPools[typeID]) {
      if (component && component->_uuid == uuid) { return component; }
    }
    rttr::type type = rttr::type::get_by_name(typeID);
    assert(type.is_valid());
    rttr::variant var = type.create();
    assert(var.is_valid());
    std::shared_ptr<MyComponent> component = (var.get_value<std::shared_ptr<MyComponent>>());
    component->_uuid = uuid;
    return std::shared_ptr<MyComponent>(component);
  }

 private:
  std::mutex                                                                 _mutex;
  std::unordered_map<std::string, std::vector<std::shared_ptr<MyComponent>>> _componentPools;
};

#endif// MYCOMPONENT_H