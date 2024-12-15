#ifndef MY3DRENDERSYSTEM_H
#define MY3DRENDERSYSTEM_H
#include "MyRenderSystem.h"
#include "rttr/registration.h"
struct MySceneManageSystem;
class My3DRenderSystem : public MyRenderSystem {
  RTTR_ENABLE();

 public:
  My3DRenderSystem(MySceneManageSystem* sceneManageSystem) : _scene_manage_system(sceneManageSystem) {}
  MySceneManageSystem* _scene_manage_system;
};

#endif // MY3DRENDERSYSTEM_H