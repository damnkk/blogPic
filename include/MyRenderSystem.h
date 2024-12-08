#ifndef MY_RENDER_SYSTEM_H
#define MY_RENDER_SYSTEM_H
class MyRenderSystem {
 public:
  virtual void update(double delta_time) {};
  virtual void draw() {};
};
#endif// MY_RENDER_SYSTEM_H