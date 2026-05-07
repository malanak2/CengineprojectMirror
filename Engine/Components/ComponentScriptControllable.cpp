#include "ComponentScriptControllable.hpp"

void Engine::ComponentScriptControllable::Setup() {}

void Engine::ComponentScriptControllable::Update() {}

void Engine::ComponentScriptControllable::FixedUpdate() {}

void Engine::ComponentScriptControllable::Save() {}

void Engine::ComponentScriptControllable::Load() {}

json Engine::ComponentScriptControllable::ToJson() {}

void Engine::ComponentScriptControllable::FromJson(json &js) {}

void Engine::ComponentScriptControllable::RenderImGui() {}

std::string Engine::ComponentScriptControllable::GetName() {
  return "Controllable";
}
