#ifndef MYRESOURCE_H
#define MYRESOURCE_H
#include "rttr/rttr_enable.h"
enum MyResourceType { Texture, Material, Program, Mesh, _COUNT };
struct MyResource {
  MyResourceType type = _COUNT;
  RTTR_ENABLE();
};

#endif// MYRESOURCE_H