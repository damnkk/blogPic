#ifndef MYFILTER_H
#define MYFILTER_H
#include <cinder/gl/gl.h>
struct MyMaterial;
struct MyFilter {
  MyFilter() = default;
  void setMaterial(std::shared_ptr<MyMaterial> material) { _Material = material; }

 private:
  cinder::gl::FboRef   _Fbo;
  cinder::gl::BatchRef _batch;

  std::shared_ptr<MyMaterial> _Material;
  MyFilter*                   _parent = nullptr;
};

#endif// MYFILTER_H