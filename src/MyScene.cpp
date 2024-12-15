#include <My2DRenderSystem.h>
#include <My3DRenderSystem.h>
#include <MyScene.h>

#include "MyApp.h"

std::atomic_uint64_t EID::_sid = 0;

void SceneNode::invalidate() {
  _parent = nullptr;
  for (auto i : _childs) {
    if (i) i->invalidate();
  }
  _childs.clear();
  _eid.invalidate();
}

void SceneNode::addChild(std::shared_ptr<SceneNode> child){
  child->_parent = this;
  child->_hostScene = this->_hostScene;
  _childs.push_back(child);
}

void SceneNode::addChild() {
  auto child = std::make_shared<SceneNode>(EID::Create());
  child->_app = _app;
  child->_parent = this;
  child->_hostScene = this->_hostScene;
  child->setName("EmptyNode_" + std::to_string(this->_childs.size()));
  this->_childs.push_back(child);
}

void SceneNode::removeChild(std::shared_ptr<SceneNode> child) {
  for (auto it = _childs.begin(); it != _childs.end(); ++it) {
    if (*it == child) {
      child->invalidate();
      child = nullptr;
      _childs.erase(it);
      return;
    }
  }
}

void SceneNode::addComponent(std::shared_ptr<MyComponent> component) {
  if (_components.find(std::string(component->get_type().get_name())) != _components.end()) { return; }
  _components[std::string(component->get_type().get_name())] = component;
  _hostScene->_renderSystem->onNotifiedAddedComponent(component.get());
};

MySceneManageSystem::MySceneManageSystem(SceneType type, MyApp *app) : _sceneType(type), _app(app) {
  _rootNode = std::make_shared<SceneNode>(EID::Create());
  _rootNode->_app = app;
  _rootNode->_hostScene = this;
  _selectedNodes.reserve(8);
  if (type == SceneType::S2D) {
    _renderSystem = std::make_shared<My2DRenderSystem>(this);
  } else {
    _renderSystem = std::make_shared<My3DRenderSystem>(this);
  }
}

void MySceneManageSystem::update(double delta_time) {
  while (!_nodeDelectionQueue.empty()) {
    auto node = _nodeDelectionQueue.front();
    _nodeDelectionQueue.pop();
    node->_parent->removeChild(node);
  }
}