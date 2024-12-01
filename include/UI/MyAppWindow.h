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

class MainBarWindow : public MyAppWindowBase {
 public:
  void Draw() override;
};

class SceneHierarchyWindow : public MyAppWindowBase {
 public:
  void Draw() override;

 private:
  static std::vector<std::shared_ptr<SceneNode>> _selectedNode;
};

class UISystem : public MyAppWindowBase {
 public:
  UISystem();
  ~UISystem();

  void Draw() override;

 private:
  std::unique_ptr<MainBarWindow>        _mainBarWindow;
  std::unique_ptr<SceneHierarchyWindow> _sceneHierarchyWindow;
};

#endif// MYAPPWINDOW_H