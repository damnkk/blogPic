#include <Component/MyFilterRenderer.h>
#include <imgui.h>

MyFilterRenderer::MyFilterRenderer() {}

MyFilterRenderer::~MyFilterRenderer() {}

void MyFilterRenderer::onGUI() {
  if (ImGui::CollapsingHeader("FilterRenderer")) { ImGui::Text("FilterRenderer"); }
}
void MyFilterRenderer::draw() {}
void MyFilterRenderer::update() {}
void MyFilterRenderer::postUpdate() {}