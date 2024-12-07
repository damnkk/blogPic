#ifndef MYSCENE_H
#define MYSCENE_H
#include <Component/MyComponent.h>
#include <MySerizalizable.h>

#include <memory>
#include <queue>
#include <vector>

struct MyApp;
struct SceneNode : public MySerializable {
  enum NodeStateFlag_ { NodeStateFlag_None = 0, NodeStateFlag_Selected = 1 << 0, NodeStateFlag_Rename = 1 << 1, NodeStateFlag_Deleted = 1 << 3 };
  SceneNode(entityx::Entity entity) : _entity(entity){};
  ~SceneNode() { _parent = nullptr; }
  void                                      invalidate();
  void                                      addChild();
  void                                      addChild(std::shared_ptr<SceneNode> child);
  void                                      removeChild(std::shared_ptr<SceneNode> child);
  std::vector<std::shared_ptr<SceneNode>>  &getChilds() { return _childs; }
  void                                      setName(const std::string &name) { _name = name; }
  SceneNode                                *_parent;
  std::vector<std::shared_ptr<SceneNode>>   _childs;
  entityx::Entity                           _entity;
  glm::mat4                                 _transform;
  std::string                               _name;
  int                                       _stateFlag = NodeStateFlag_None;
  MyApp                                    *_app;
};

struct MySceneManageSystem : public entityx::System<MySceneManageSystem> {
  enum SceneType { S2D, S3D };
  MySceneManageSystem(SceneType sceneType, MyApp *app);
  void                                    update(entityx::EntityManager &entities, entityx::EventManager &events, entityx::TimeDelta dt) override;
  std::shared_ptr<SceneNode> _rootNode;
  SceneType                  _sceneType = SceneType::S2D;
  std::vector<std::shared_ptr<SceneNode>> _selectedNodes;
  std::queue<std::shared_ptr<SceneNode>>  _nodeDelectionQueue;
  MyApp                     *_app;
};

#endif // MYSCENE_H