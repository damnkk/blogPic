#include <Component/MyFilterRenderer.h>
#include <Resource/MyMaterial.h>
#include <imgui.h>

#include "MyApp.h"
#include "MyFilter.h"

MyFilterRenderer::MyFilterRenderer(MyApp* app) : _app(app) {}

MyFilterRenderer::~MyFilterRenderer() {}

void MyFilterRenderer::draw() {}
void MyFilterRenderer::update(double deltaTime) {}
void MyFilterRenderer::postUpdate() {}
void MyFilterRenderer::addFilter(std::string name) { auto filter = _filters.emplace_back(std::make_shared<MyFilter>(name)); }

cinder::Json MyFilterRenderer::transfer() {
  cinder::Json res;
  for (auto& filter : _filters) { res[filter->getName()] = filter->transfer(); }
  return res;
}

void MyFilterRenderer::transfer(cinder::Json& json) {}
