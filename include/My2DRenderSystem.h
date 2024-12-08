#ifndef MY2DRENDERSYSTEM_H
#define MY2DRENDERSYSTEM_H
#include <MyScene.h>

#include "MyRenderSystem.h"

class My2DRenderSystem : public MyRenderSystem {
 public:
  My2DRenderSystem(MySceneManageSystem *scene_manage_system) : _scene_manage_system(scene_manage_system) {}
  void update(double delta_time) override;
  void draw() override;

 private:
  MySceneManageSystem *_scene_manage_system;
};

#endif// MY2DRENDERSYSTEM_H