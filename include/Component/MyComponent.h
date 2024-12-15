#ifndef MYCOMPONENT_H
#define MYCOMPONENT_H
#include <cinder/gl/gl.h>
#include <rttr/rttr_enable.h>
struct MyComponent {
  virtual void preUpdate() {};
  virtual void update(double deltaTime) {};
  virtual void draw() {};
  virtual void postUpdate() {};
  RTTR_ENABLE()
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

#endif// MYCOMPONENT_H