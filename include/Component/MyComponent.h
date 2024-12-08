#ifndef MYCOMPONENT_H
#define MYCOMPONENT_H
#include <cinder/gl/gl.h>
#include <rttr/rttr_enable.h>
struct MyComponent {
  virtual void onGUI() {};
  virtual void preUpdate() {};
  virtual void update() {};
  virtual void draw() {};
  virtual void postUpdate() {};
  RTTR_ENABLE()
};

struct ScreenRenderable : public MyComponent {
  cinder::gl::BatchRef                 batch;
  std::shared_ptr<cinder::CameraPersp> camera;
  RTTR_ENABLE(MyComponent)
};

#endif// MYCOMPONENT_H