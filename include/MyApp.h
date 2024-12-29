#ifndef MYAPP_H
#include <MyProject.h>
#include <MyScene.h>
#define IMGUI_DEFINE_MATH_OPERATORS_IMPLEMENTED
#include <cinder/Color.h>
#include <cinder/Json.h>
#include <cinder/Log.h>
#include <cinder/Thread.h>
#include <cinder/app/App.h>
#include <cinder/app/AppBase.h>
#include <cinder/app/KeyEvent.h>
#include <cinder/app/RendererGl.h>

#include "MyRenderSystem.h"

struct UISystem;
struct MyProject;
struct FileBroserWindow;

struct MyApp : public cinder::app::App {
 public:
  void keyDown(cinder::app::KeyEvent event) override;
  void draw() override;
  void update() override;
  void setup() override;
  // void cleanup() override;
 public:
  void createProject(std::string basePath, MyProject::ProjectType projectType);
  void loadProject(const cinder::Json& proj);
  void adjustForDPI();
  void checkUpdateShaders();
  double getFrameDeltaTime();

 public:
  std::shared_ptr<UISystem>            _uiSystem;
  std::shared_ptr<MyProject>           _myProject;
  std::shared_ptr<MySceneManageSystem> _myScene;
  bool                                 _newProject = false;

 private:
  cinder::log::LoggerRef _logger;
  enum PopupState { NewProj, WhatEver, None };
  void       load2DProject(const cinder::Json& projJson);
  void       load3DProject(const cinder::Json& projJson);
  std::shared_ptr<MyRenderSystem> _renderSystem;
  PopupState                      _popupState = PopupState::None;
  double     _lastTime = 0;
};

#endif // MYAPP_H