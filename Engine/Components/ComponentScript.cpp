#include "ComponentScript.hpp"
#include "Interfaces/IComponent.hpp"

Engine::ENGINE_COMPONENT_TYPE Engine::ComponentScript::GetType() {
  return ENGINE_COMPONENT_TYPE::script;
}

std::string Engine::ComponentScript::GetName() {
  return "NOT IMPLEMENTED - Implement std::string ComponentScript::GetName()";
}
