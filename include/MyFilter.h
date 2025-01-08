#ifndef MYFILTER_H
#define MYFILTER_H
#include <cinder/gl/gl.h>
#include <cinder/Json.h>
#include <Resource/MyResource.h>
#include <component/MyComponent.h>
struct MyMaterial;

struct MyFilter : public MyComponent {
  MyFilter(const std::string& name, int width = 1920, int height = 1080);
  virtual ~MyFilter() {}
  void setMaterial(std::shared_ptr<MyMaterial> material);
  std::string  getName() const { return _name; }
  cinder::Json transfer() override;

 private:
  cinder::gl::VboMeshRef getQuadMesh() const;
  MyRenderTexture        _rt;

  std::shared_ptr<MyMaterial> _material;
  std::vector<MyFilter*>      _parent;
  std::string                 _name;
  glm::ivec2                  _size;
};

#endif// MYFILTER_H