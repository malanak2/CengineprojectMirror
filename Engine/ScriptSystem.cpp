#include "ScriptSystem.hpp"
namespace Engine::Main {
bool Engine::Main::ScriptSystem::RegisterScript(
    std::shared_ptr<ComponentScript> script) {
  bool ret = map_comp.contains(script->GetName());
  map_comp[script->GetName()] = script;
  return ret;
}
std::shared_ptr<ComponentScript> ScriptSystem::GetScript(std::string_view key) {
  if (!map_comp.contains(key))
    return nullptr;
  return this->map_comp[key];
}
} // namespace Engine::Main
