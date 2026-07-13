#include "ComponentScript.hpp"
#include "Interfaces/IComponent.hpp"
#include "imgui.h"

Engine::ENGINE_COMPONENT_TYPE Engine::ComponentScript::GetType() {
  return ENGINE_COMPONENT_TYPE::script;
}

void Engine::ComponentScript::RenderImGui() {
  ImGui::Text(
      "NOT IMPLEMENTED - Implement void ComponentScript::RenderImGui()");
}
std::string Engine::ComponentScript::GetName() {
  return "NOT IMPLEMENTED - Implement std::string ComponentScript::GetName()";
}
