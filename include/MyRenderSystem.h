#ifndef MY_RENDER_SYSTEM_H
#define MY_RENDER_SYSTEM_H

struct MyComponent;
class MyRenderSystem {
 public:
  virtual void update(double delta_time) {};
  virtual void draw() {};
  virtual void onNotifiedAddedComponent(MyComponent* component) {};
};
#endif// MY_RENDER_SYSTEM_H