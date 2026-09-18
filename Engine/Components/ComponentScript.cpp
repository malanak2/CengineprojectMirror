#include "ComponentScript.hpp"
#include "Interfaces/IComponent.hpp"

Engine::ENGINE_COMPONENT_TYPE Engine::ComponentScript::GetType() {
  return ENGINE_COMPONENT_TYPE::script;
}
