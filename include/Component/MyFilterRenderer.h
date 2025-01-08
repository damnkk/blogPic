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
  std::vector<std::shared_ptr<MyFilter>>& getFilters() { return _filters; };
  cinder::Json                            transfer() override;
  void                                    transfer(cinder::Json& json) override;
  RTTR_ENABLE(MyRenderable)
 protected:
  std::vector<std::shared_ptr<MyFilter>> _filters;
  MyApp*                                 _app;
  bool                                   _isEnable = true;
};

#endif// MY_FILTER_RENDERER_H