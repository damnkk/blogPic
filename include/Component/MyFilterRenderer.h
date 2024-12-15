#ifndef MY_FILTER_RENDERER_H
#define MY_FILTER_RENDERER_H
#include <Component/MyComponent.h>
struct MyFilter;
struct MyApp;
class MyFilterRenderer : public MyRenderable {
 public:
  MyFilterRenderer() = default;
  MyFilterRenderer(MyApp* app);
  virtual ~MyFilterRenderer();
  void draw() override;
  void update(double deltaTime) override;
  void postUpdate() override;
  void addFilter(std::string name);
  RTTR_ENABLE(MyRenderable)
 protected:
  std::vector<std::shared_ptr<MyFilter>> _filters;
  MyApp*                                 _app;
};

#endif// MY_FILTER_RENDERER_H