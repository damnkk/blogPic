#ifndef MYSCENE_H
#define MYSCENE_H
#include <vector>
#include <memory>
#include <MyComponent.h>

struct SceneNode{
    void addChild(std::shared_ptr<SceneNode> child);
    std::shared_ptr<SceneNode> _parent;
    std::vector<std::shared_ptr<SceneNode>> _childs;
    std::vector<std::shared_ptr<MyComponent>> _components;

};

struct MyScene{
    enum SceneType{
        S2D,
        S3D
    };
    std::shared_ptr<SceneNode> _rootNode;
    SceneType _sceneType=SceneType::S2D;
};

#endif // MYSCENE_H