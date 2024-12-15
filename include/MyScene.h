#ifndef MYSCENE_H
#define MYSCENE_H
#include <Component/MyComponent.h>
#include <MySerizalizable.h>

#include <memory>
#include <queue>
#include <vector>

struct MyApp;
struct MyRenderSystem;
struct MySceneManageSystem;
struct EID {
  static std::atomic_uint64_t _sid;
  static EID                  Create() { return {_sid++}; };
  bool                        valid() { return _id != 0; }
  void                        invalidate() { _id = 0; }
  uint64_t                    _id = 0;
};

struct SceneNode : public MySerializable {
  enum NodeStateFlag_ { NodeStateFlag_None = 0, NodeStateFlag_Selected = 1 << 0, NodeStateFlag_Rename = 1 << 1, NodeStateFlag_Deleted = 1 << 3 };
  SceneNode(EID id) : _eid(id){};
  ~SceneNode() { _parent = nullptr; }
  void                                      invalidate();
  void                                      addChild();
  void                                      addChild(std::shared_ptr<SceneNode> child);
  void                                      removeChild(std::shared_ptr<SceneNode> child);
  void                                                          addComponent(std::shared_ptr<MyComponent> component);
  std::vector<std::shared_ptr<SceneNode>>  &getChilds() { return _childs; }
  void                                      setName(const std::string &name) { _name = name; }
  SceneNode                                *_parent;
  std::vector<std::shared_ptr<SceneNode>>   _childs;
  std::unordered_map<std::string, std::shared_ptr<MyComponent>> _components;
  EID                                                           _eid;
  glm::mat4                                 _transform;
  std::string                               _name;
  int                                       _stateFlag = NodeStateFlag_None;
  MyApp                                                        *_app;
  MySceneManageSystem                                          *_hostScene;
};

struct MySceneManageSystem {
  enum SceneType { S2D, S3D };
  MySceneManageSystem(SceneType sceneType, MyApp *app);
  void                                    update(double dt);
  std::shared_ptr<SceneNode> _rootNode;
  SceneType                  _sceneType = SceneType::S2D;
  std::vector<std::shared_ptr<SceneNode>> _selectedNodes;
  std::queue<std::shared_ptr<SceneNode>>  _nodeDelectionQueue;
  MyApp                                  *_app;
  std::shared_ptr<MyRenderSystem>         _renderSystem;
};

#endif // MYSCENE_H