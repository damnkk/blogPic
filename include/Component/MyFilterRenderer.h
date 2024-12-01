#ifndef MY_FILTER_RENDERER_H
#define MY_FILTER_RENDERER_H
#include <Component/MyComponent.h>
struct MyFilter;

class MyFilterRenderer : public MyComponent {
 public:
  MyFilterRenderer();
  ~MyFilterRenderer();
  void draw() override;
  void update() override;
  void postUpdate() override;

 protected:
  std::vector<MyFilter*> _filters;
};

#endif// MY_FILTER_RENDERER_H