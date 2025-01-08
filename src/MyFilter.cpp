#include <MyFilter.h>

#include "Resource/MyMaterial.h"
#include <immintrin.h>

MyFilter::MyFilter(const std::string &name, int width, int height) {
  this->_name = name;
  this->_size.x = width;
  this->_size.y = height;

  cinder::gl::Fbo::Format fmt{};
  _rt = MyRenderTexture(width, height, fmt);
}

cinder::gl::VboMeshRef MyFilter::getQuadMesh() const {
  static cinder::gl::VboMeshRef mesh = cinder::gl::VboMesh::create(cinder::geom::Rect());
  return mesh;
}

void MyFilter::setMaterial(std::shared_ptr<MyMaterial> material) {
  if (_material == material) { return; }
  _material = material;
  // _batch = cinder::gl::Batch::create(getQuadMesh(), _material->getProgram());
}

cinder::Json MyFilter::transfer() {
  cinder::Json res;
  res["name"] = _name;
  res["size"] = cinder::Json::array({_size.x, _size.y});
  res["material"] = _material->transfer();

  return res;
}