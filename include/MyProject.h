#ifndef MYPROJECT_H
#define MYPROJECT_H
#include <unordered_map>
#include <cinder/gl/gl.h>

struct MyProject{
  enum class ProjectType { Proj2D, Proj3D, _Count };

  ProjectType                                              type;
  std::unordered_map<std::string, cinder::gl::GlslProgRef> shadersMap;
  std::string                                              basePath;
  bool                                                     isLoaded = false;
};




#endif // MYPROJECT_H

