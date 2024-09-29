#ifndef MYPROJECT_H
#define MYPROJECT_H
#include <unordered_map>
#include <cinder/gl/gl.h>

struct MyProject{
    enum ProjectType{
        ShaderToy,
        ThreeDPlayGround
    };

    ProjectType type;
    std::unordered_map<std::string,cinder::gl::GlslProgRef> shadersMap;
    std::string basePath;
    bool isLoaded = false;
};




#endif // MYPROJECT_H

