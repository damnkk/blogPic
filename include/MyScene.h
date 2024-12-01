#ifndef MYSCENE_H
#define MYSCENE_H
#include <Component/MyComponent.h>
#include <MySerizalizable.h>

#include <memory>
#include <vector>

struct MyApp;
struct SceneNode : public MySerializable {
  SceneNode(entityx::Entity entity) : _entity(entity){};
  ~SceneNode() { _entity.destroy(); }
  void                                      addChild();
  void                                      addChild(std::shared_ptr<SceneNode> child);
  std::vector<std::shared_ptr<SceneNode>>  &getChilds() { return _childs; }
  void                                      setName(const std::string &name) { _name = name; }
  SceneNode                                *_parent;
  std::vector<std::shared_ptr<SceneNode>>   _childs;
  std::vector<std::shared_ptr<MyComponent>> _components;
  entityx::Entity                            _entity;
  glm::mat4                                  _transform;
  std::string                                _name;
  MyApp                                     *_app;
};

struct MySceneManageSystem : public entityx::System<MySceneManageSystem> {
  enum SceneType { S2D, S3D };
  MySceneManageSystem(SceneType sceneType, MyApp *app);
  void                       update(entityx::EntityManager &entities, entityx::EventManager &events, entityx::TimeDelta dt) override {};
  std::shared_ptr<SceneNode> _rootNode;
  SceneType                  _sceneType = SceneType::S2D;
  MyApp                     *_app;
};

#endif // MYSCENE_H