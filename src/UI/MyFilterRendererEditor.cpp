#include <UI/MyFilterRendererEditor.h>
#include <cinder/CinderImGui.h>
#include <component/MyFilterRenderer.h>

void MyFilterRendererEditor::OnGUI(MyComponent* component) {
  MyFilterRenderer* filterrenderer = static_cast<MyFilterRenderer*>(component);

  if (ImGui::CollapsingHeader("MyFilterRenderer")) {
    static char test[64];
    ImGui::InputText("FilterName", test, 64);
    filterrenderer->addFilter(test);
  }
}