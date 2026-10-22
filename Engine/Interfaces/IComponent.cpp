#include "IComponent.hpp"
#include "Engine.hpp"

namespace Engine {
void IComponent::EarlySetup() {}
void IComponent::Setup() {}
void IComponent::Update() {}
void IComponent::End() {}
void IComponent::LateEnd() {}

float IComponent::GetDeltaTime() { return Engine::Engine::GetDeltaTime(); }

void IComponent::FixedUpdate() {}
void IComponent::SetObject(std::shared_ptr<Object> object) {
  this->object = object;
}

} // namespace Engine
