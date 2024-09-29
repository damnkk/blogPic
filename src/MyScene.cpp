#include <MyScene.h>

void SceneNode::addChild(std::shared_ptr<SceneNode> child){
    child->_parent = std::shared_ptr<SceneNode>(this);
    _childs.push_back(child);
}