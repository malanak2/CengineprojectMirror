#include "ScriptSystem.hpp"
#include <typeindex>
namespace Engine::Main {
bool Engine::Main::ScriptSystem::RegisterScript(
    std::shared_ptr<ComponentScript> script) {
  auto ind = std::type_index(typeid(script)).name();
  bool ret = map_comp.contains(ind);
  map_comp[ind] = script;
  return ret;
}
std::shared_ptr<ComponentScript> ScriptSystem::GetScript(std::string key) {
  if (!map_comp.contains(key))
    return nullptr;
  return this->map_comp[key];
}
} // namespace Engine::Main
