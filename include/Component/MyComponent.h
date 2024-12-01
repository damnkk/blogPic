#ifndef MYCOMPONENT_H
#define MYCOMPONENT_H
#include <cinder/gl/gl.h>
#include <entityx/entityx.h>

struct MyComponent : public entityx::Component<MyComponent> {
  virtual void preUpdate() {};
  virtual void update() {};
  virtual void draw() {};
  virtual void postUpdate() {};
};

struct ScreenRenderable : public MyComponent {
  cinder::gl::BatchRef                 batch;
  std::shared_ptr<cinder::CameraPersp> camera;
};

#endif// MYCOMPONENT_H