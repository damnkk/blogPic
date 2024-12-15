#ifndef MYREGISTRATION_H
#define MYREGISTRATION_H
#include "MyRegistration.h"

#include <rttr/registration.h>

RTTR_REGISTRATION {
  using namespace rttr;
  registration::class_<MyComponent>("MyComponent").constructor<>();
  registration::class_<MyFilterRenderer>("MyFilterRenderer")
      .constructor<>()
      .method("draw", &MyFilterRenderer::draw)
      .method("update", &MyFilterRenderer::update)
      .method("postUpdate", &MyFilterRenderer::postUpdate);
  registration::class_<MyComponentEditorBase>("MyComponentEditorBase").constructor<>();
  registration::class_<MyFilterRendererEditor>("MyFilterRendererEditor").constructor<>().method("OnGUI", &MyFilterRendererEditor::OnGUI);
}
#endif// MYREGISTRATION_H