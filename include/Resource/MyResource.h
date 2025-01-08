#ifndef MYRESOURCE_H
#define MYRESOURCE_H
#include "rttr/rttr_enable.h"
#include <uuid.h>
#include <MySerizalizable.h>
#include <cinder/Cinder.h>
#include <cinder/gl/gl.h>
enum MyResourceType { Texture, Material, Program, Mesh, _COUNT };
struct MyResource : public MySerializable {
  MyResource() = default;
  MyResource(uuids::uuid id) : _uid(id) {}
  MyResourceType type = _COUNT;
  uuids::uuid    _uid;
  RTTR_ENABLE();
};

class MyMesh : public MyResource {
  MyMesh() { type = Mesh; }

 private:
  cinder::gl::VboMeshRef _mesh;

  RTTR_ENABLE(MyResource)
};
class MyTexture : public MyResource {
  MyTexture() { type = Texture; }
  cinder::gl::Texture2dRef _texture;
  RTTR_ENABLE(MyResource)
};

class MyRenderTexture : public MyResource {
 public:
  MyRenderTexture() {}
  MyRenderTexture(uint32_t width, uint32_t height, cinder::gl::Fbo::Format format) { type = Texture; }
  cinder::gl::FboRef _fbo;
  RTTR_ENABLE(MyResource)
};

class MyProgram : public MyResource {
  MyProgram() { type = Program; }
  cinder::gl::GlslProgRef _shaders;
  RTTR_ENABLE(MyResource)
};

#endif// MYRESOURCE_H