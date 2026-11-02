#include "TestComponent.hpp"
#include "Interfaces/IComponent.hpp"
#include "JsonFileBase.hpp"
#include "Object.hpp"
void Engine::TestComponent::Update() {
  auto o = object.lock();
  o->AddPosition(glm::vec3{0, amount, 0});
}
