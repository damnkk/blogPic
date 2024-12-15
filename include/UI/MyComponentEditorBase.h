#ifndef MYCOMPONENTEDITORBASE_H
#define MYCOMPONENTEDITORBASE_H
#include <rttr/rttr_enable.h>
struct MyComponent;
class MyComponentEditorBase {
  RTTR_ENABLE();

 public:
  virtual void OnGUI(MyComponent* component) {};
};

class MyEditorFactory {
 public:
  static MyEditorFactory* instance() {
    static MyEditorFactory factory = MyEditorFactory();
    return &factory;
  }
  virtual ~MyEditorFactory();
  rttr::variant getOrCreateEditor(const std::string& name);

 private:
  std::unordered_map<std::string, rttr::variant> _editorsMap;
};

#endif// MYCOMPONENTEDITORBASE_H