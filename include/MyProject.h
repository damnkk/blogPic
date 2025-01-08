#ifndef MYPROJECT_H
#define MYPROJECT_H
#include <unordered_map>
#include <cinder/gl/gl.h>
#include <uuid.h>
#include <Resource/MyResource.h>

struct MyApp;
class AssetManager {
 public:
  static AssetManager* instance() {
    static AssetManager instance;
    return &instance;
  }
  std::shared_ptr<MyResource> createAssetByName(const std::string& name);
  void                        init(std::string path) { _basePath = path; }

 private:
  std::unordered_map<std::string, std::vector<std::shared_ptr<MyResource>>>   _resourceMap;
  std::unordered_map<std::string, uuids::uuid> _pathToUUID;
  std::unordered_map<uuids::uuid, std::shared_ptr<MyResource>>              _uuidToResource;
  std::string                                                               _basePath;
};
struct MyProject{
  enum class ProjectType { Proj2D, Proj3D, _Count };
  void                                                     save();
  ProjectType                                              type;
  std::unordered_map<std::string, cinder::gl::GlslProgRef> shadersMap;
  std::string                                              basePath;
  bool                                                     isLoaded = false;
  MyApp*                                                   app = nullptr;
};




#endif // MYPROJECT_H

