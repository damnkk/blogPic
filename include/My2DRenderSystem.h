#ifndef MY2DRENDERSYSTEM_H
#define MY2DRENDERSYSTEM_H
#include <MyScene.h>
#include <entityx/entityx.h>

class My2DRenderSystem : public entityx::System<My2DRenderSystem> {
 public:
  My2DRenderSystem(MySceneManageSystem *scene_manage_system) : _scene_manage_system(scene_manage_system) {}
  void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override;
  void draw();

 private:
  MySceneManageSystem *_scene_manage_system;
};

#endif// MY2DRENDERSYSTEM_H