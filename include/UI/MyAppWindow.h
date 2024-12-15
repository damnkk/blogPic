#ifndef MYAPPWINDOW_H
#include <imgui.h>

#include <filesystem>
#include <string>
#include <vector>
struct SceneNode;
struct MyApp;
class MyAppWindowBase {
 public:
  virtual void Draw() = 0;
  static void  init(MyApp* app) { _app = app; };

 protected:
  ImVec2        windowPos;
  ImVec2        windowSize;
  static MyApp* _app;
};
struct UISystem;
class MainBarWindow : public MyAppWindowBase {
 public:
  MainBarWindow(UISystem* uiSystem) : _uiSystem(uiSystem){};
  void Draw() override;

 private:
  UISystem* _uiSystem;
};

class SceneHierarchyWindow : public MyAppWindowBase {
 public:
  SceneHierarchyWindow(UISystem* uiSystem) : _uiSystem(uiSystem){};
  void Draw() override;

 private:
  void      DrawRecursiveNode(std::shared_ptr<SceneNode> node);
  UISystem* _uiSystem;
};

class NodeInspectorWindow : public MyAppWindowBase {
 public:
  NodeInspectorWindow(UISystem* uiSystem) : _uiSystem(uiSystem){};
  void Draw() override;

 private:
  friend class SceneHierarchyWindow;
  UISystem* _uiSystem;
};

class ResourceManageWindow : public MyAppWindowBase {
 public:
  ResourceManageWindow(UISystem* uiSystem) : _uiSystem(uiSystem){};
  void Draw() override;

 private:
  UISystem* _uiSystem;
};

class UISystem : public MyAppWindowBase {
 public:
  UISystem();
  ~UISystem();

  void Draw() override;
  void update();

 private:
  friend class MainBarWindow;
  friend class SceneHierarchyWindow;
  friend class NodeInspectorWindow;
  std::unique_ptr<MainBarWindow>        _mainBarWindow;
  std::unique_ptr<SceneHierarchyWindow> _sceneHierarchyWindow;
  std::unique_ptr<NodeInspectorWindow>    _nodeInspectorWindow;
  std::unique_ptr<ResourceManageWindow>   _resourceManageWindow;
  std::vector<std::shared_ptr<SceneNode>> _selectedNode;
};

#endif// MYAPPWINDOW_H