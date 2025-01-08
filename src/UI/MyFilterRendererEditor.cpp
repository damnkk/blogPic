#include <UI/MyFilterRendererEditor.h>
#include <cinder/CinderImGui.h>
#include <component/MyFilterRenderer.h>
#include <MyFilter.h>

void MyFilterRendererEditor::OnGUI(MyComponent* component) {
  MyFilterRenderer* filterrenderer = static_cast<MyFilterRenderer*>(component);

  if (ImGui::CollapsingHeader("MyFilterRenderer")) {
    static char test[64];
    ImGui::PushID("FilterNameInput");
    ImGui::InputText("", test, 64);
    ImGui::PopID();
    ImGui::SameLine();
    if (ImGui::Button("Add Filter")) { filterrenderer->addFilter(test); }
    for (auto& filter : static_cast<MyFilterRenderer*>(component)->getFilters()) {
      if (ImGui::TreeNode(filter->getName().c_str())) { ImGui::TreePop(); }
    }
  }
}