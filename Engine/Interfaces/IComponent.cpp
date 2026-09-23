#include "IComponent.hpp"
#include "Engine.hpp"

namespace Engine {
/*std::map<ENGINE_COMPONENT_TYPE, void(json &js)> constructors = {
    {ENGINE_COMPONENT_TYPE::renderable,
     Graphics::ComponentRenderable::ComponentRenderable},
   {ENGINE_COMPONENT_TYPE::camera, Graphics::CameraComponent::FromJson}};*/
void IComponent::Setup() {}
void IComponent::Update() {}
void IComponent::End() {}
float IComponent::GetDeltaTime() { return Engine::Engine::GetDeltaTime(); }

void IComponent::FixedUpdate() {}
void IComponent::SetObject(std::shared_ptr<Object> object) {
  this->object = object;
}

} // namespace Engine
