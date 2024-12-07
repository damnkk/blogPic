#include <MyScene.h>

#include "MyApp.h"

void SceneNode::invalidate() {
  _parent = nullptr;
  for (auto i : _childs) {
    if (i) i->invalidate();
  }
  _childs.clear();
  _entity.invalidate();
}

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

void SceneNode::removeChild(std::shared_ptr<SceneNode> child) {
  for (auto it = _childs.begin(); it != _childs.end(); ++it) {
    if (*it == child) {
      child->invalidate();
      _childs.erase(it);
      return;
    }
  }
}

MySceneManageSystem::MySceneManageSystem(SceneType type, MyApp *app) : _sceneType(type), _app(app) {
  _rootNode = std::make_shared<SceneNode>(_app->entities.create());
  _rootNode->_app = app;
  _selectedNodes.reserve(8);
}

void MySceneManageSystem::update(entityx::EntityManager &entities, entityx::EventManager &events, entityx::TimeDelta dt) {
  while (!_nodeDelectionQueue.empty()) {
    auto node = _nodeDelectionQueue.front();
    _nodeDelectionQueue.pop();
    node->_parent->removeChild(node);
  }
}