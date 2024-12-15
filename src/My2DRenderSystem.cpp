#include <Component/MyComponent.h>
#include <Component/MyFilterRenderer.h>
#include <My2DRenderSystem.h>

void My2DRenderSystem::update(double dt) {
  // Update all entities with a Renderable component
  // for (auto entity : es.entities_with_components(Renderable)) {
  //     auto &renderable = entity.component<Renderable>();
}

void My2DRenderSystem::onNotifiedAddedComponent(MyComponent* component) {
  if (rttr::type::get(*component).is_derived_from<MyRenderable>()) { _rendererList.push_back((MyFilterRenderer*) component); }
}

void My2DRenderSystem::draw() {}