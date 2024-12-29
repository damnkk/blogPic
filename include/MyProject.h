#ifndef MYPROJECT_H
#define MYPROJECT_H
#include <unordered_map>
#include <cinder/gl/gl.h>
#include <uuid.h>
#include <Resource/MyResource.h>

struct MyApp;
class AssetManager {
 public:
  void createAsset(MyResourceType type);

 private:
  std::unordered_map<std::string, uuids::uuid> _pathToUUID;
  std::unordered_map<uuids::uuid, MyResource*> _uuidToResource;
};
struct MyProject{
  enum class ProjectType { Proj2D, Proj3D, _Count };
  void                                                     save() {};
  ProjectType                                              type;
  std::unordered_map<std::string, cinder::gl::GlslProgRef> shadersMap;
  std::string                                              basePath;
  bool                                                     isLoaded = false;
  MyApp*                                                   app = nullptr;
};




#endif // MYPROJECT_H

