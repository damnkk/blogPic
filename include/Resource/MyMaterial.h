#ifndef MYMATERIAL_H
#define MYMATERIAL_H
#include <cinder/Cinder.h>
#include <cinder/gl/gl.h>
#include <rttr/variant.h>
#include <resource/MyResource.h>

class MyMaterial : public MyResource {
  RTTR_ENABLE()
 public:
  void setProgram(cinder::gl::GlslProgRef shader) { _shader = shader; }
  void setUniform(const std::string& name, const rttr::variant& value);
  cinder::gl::GlslProgRef getProgram() const { return _shader; }

 private:
  std::unordered_map<std::string, rttr::variant> _ParamMap;
  cinder::gl::GlslProgRef                        _shader;
};

#endif// MYMATERIAL_H