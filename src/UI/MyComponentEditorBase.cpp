#include "UI/MyComponentEditorBase.h"

#include "Component/MyFilterRenderer.h"

MyEditorFactory::~MyEditorFactory() {}

rttr::variant MyEditorFactory::getOrCreateEditor(const std::string& name) {
  rttr::type type = rttr::detail::get_invalid_type();
  if ("MyFilterRenderer" == name) { type = rttr::type::get_by_name("MyFilterRendererEditor"); }
  if (_editorsMap.find(type.get_name().to_string()) == _editorsMap.end()) {
    _editorsMap[type.get_name().to_string()] = type.create({});
  } else {
    return _editorsMap[type.get_name().to_string()];
  }
  return nullptr;
}