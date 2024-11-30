#ifndef MYAPP_H
#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <cinder/app/AppBase.h>
#include <cinder/app/KeyEvent.h>
#include <cinder/Log.h>
#include <cinder/Color.h>
#include <cinder/CinderImGui.h>
#include <cinder/CinderImGuiConfig.h>
#include <cinder/Json.h>
#include <cinder/Thread.h>
#include <MyScene.h>
#include <ecs/Event.h>
#include <ecs/Entity.h>
#include <MyWindowManager.h>
#include <core/MyMemoryAllocator.h>

struct MyProject;
struct FileBroserWindow;

struct MyApp:public cinder::app::App{
public:
void keyDown(cinder::app::KeyEvent event) override;
void draw() override;
void update() override;
void setup() override;
// void cleanup() override;
public:
void createProject(std::string basePath);
void loadProject(const cinder::Json& proj);
void adjustForDPI();
void checkUpdateShaders();
public:
    std::shared_ptr<FileBroserWindow> _fileBroserWindow;
    std::shared_ptr<MyProject> _myProject;
    std::shared_ptr<MyScene> _myScene;
    bool _newProject = false;
    WindowManager manager;
private:
    enum PopupState{
        NewProj,
        WhatEver,
        None
    };
    void load2DProject(const cinder::Json& projJson);
    void load3DProject();
    PopupState _popupState=PopupState::None;
};

#endif // MYAPP_H