#include "TestComponent.hpp"
#include "Interfaces/IComponent.hpp"
#include "JsonFileBase.hpp"
#include "Object.hpp"
void Engine::TestComponent::Update() {
  auto o = object.lock();
  o->_position = {o->_position[0], o->_position[1] + amount, o->_position[2]};
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
