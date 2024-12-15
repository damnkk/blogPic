#ifndef MYFILTER_H
#define MYFILTER_H
#include <cinder/gl/gl.h>
struct MyMaterial;
struct MyFilter {
  MyFilter() = default;
  MyFilter(const std::string& name, int width = 1920, int height = 1080);
  virtual ~MyFilter() {}
  void setMaterial(std::shared_ptr<MyMaterial> material);

 private:
  cinder::gl::VboMeshRef getQuadMesh() const;
  cinder::gl::FboRef   _Fbo;
  cinder::gl::BatchRef _batch;

  std::shared_ptr<MyMaterial> _material;
  std::vector<MyFilter*>      _parent;
  std::string                 _name;
  glm::ivec2                  _size;
};

#endif// MYFILTER_H