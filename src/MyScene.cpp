#include <MyScene.h>

#include "MyApp.h"

void SceneNode::addChild(std::shared_ptr<SceneNode> child){
  child->_parent = this;
  _childs.push_back(child);
}

void SceneNode::addChild() {
  auto child = std::make_shared<SceneNode>(_app->entities.create());
  child->_app = _app;
  child->_parent = this;
  child->setName("EmptyNode_" + std::to_string(this->_childs.size()));
  this->_childs.push_back(child);
}

MySceneManageSystem::MySceneManageSystem(SceneType type, MyApp *app) : _sceneType(type), _app(app) {
  _rootNode = std::make_shared<SceneNode>(_app->entities.create());
  _rootNode->_app = app;
}