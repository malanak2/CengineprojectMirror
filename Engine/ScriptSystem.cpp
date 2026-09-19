#include "ScriptSystem.hpp"
#include "Components/InvalidComponent.hpp"
#include "Util/LoggerUtil.hpp"
#include <memory>
namespace Engine::Main {
std::shared_ptr<ScriptSystem> ScriptSystem::instance =
    std::make_shared<ScriptSystem>();
std::shared_ptr<IComponent> ScriptSystem::GetScript(std::string_view key) {
  auto it = map_comp.find(std::string(key));
  if (it != map_comp.end()) {
    return it->second(); // Executes lambda -> fresh instance
  }
  SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Unregistered key found in scene: {}",
                      std::string(key));
  return std::make_shared<InvalidComponent>();
}
} // namespace Engine::Main
