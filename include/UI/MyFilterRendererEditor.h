#ifndef MYFILTERRENDEREREDITOR_H
#define MYFILTERRENDEREREDITOR_H

#include "MyComponentEditorBase.h"

class MyFilterRendererEditor : public MyComponentEditorBase {
  RTTR_ENABLE(MyComponentEditorBase);

 public:
  void OnGUI(MyComponent* component) override;
};

#endif// MYFILTERRENDEREREDITOR_H