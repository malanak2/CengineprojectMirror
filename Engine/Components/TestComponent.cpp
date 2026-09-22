#include "TestComponent.hpp"
#include "Interfaces/IComponent.hpp"
#include "JsonFileBase.hpp"
#include "Object.hpp"
void Engine::TestComponent::Update() {
  auto o = object.lock();
  o->_position = o->_position + glm::vec3{0, amount, 0};
}
json Engine::TestComponent::ToJson() {
  TestComponentJson t;
  t.amount = amount;
  return t;
}
void Engine::TestComponent::FromJson(json &js) {
  TestComponentJson t = js;
  amount = t.amount;
}
