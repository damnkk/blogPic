#ifndef MYMATERIAL_H
#define MYMATERIAL_H
#include <Utility/MyVariant.h>
#include <cinder/Cinder.h>
#include <cinder/gl/gl.h>
#include <rttr/variant.h>

class MyMaterial {
 public:
  void setProgram(cinder::gl::GlslProgRef shader) { _shader = shader; }
  void setUniform(const std::string& name, const rttr::variant& value);

 private:
  std::unordered_map<std::string, rttr::variant> _ParamMap;
  cinder::gl::GlslProgRef                        _shader;
};

#endif// MYMATERIAL_H