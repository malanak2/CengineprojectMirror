#pragma once
#include "Interfaces/IComponent.hpp"
#include <map>
#include <memory>
namespace Engine {
namespace Main {
/*     if (engineInstance->scriptSystem->RegisterScript(script)) { \
 *     engineInstance->ReloadScene; \
 *   } \
 */
//!
//! @def REGISTER_SCRIPT(script) Registers the @script specified into the system
//!
#define REGISTER_SCRIPT(script)                                                \
  {                                                                            \
    (Main::ScriptSystem::instance->RegisterScript<script>());                  \
  }

//!
//! @brief System for registering user scripts. Uses the std::type_index
//!
class ScriptSystem {
public:
  static std::shared_ptr<ScriptSystem> instance;
  template <typename T> void RegisterScript() {
    SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Registering script {}",
                       T::GetNameS());
    map_comp[std::string(T::GetNameS())] = []() -> std::shared_ptr<IComponent> {
      return std::make_shared<T>();
    };
  }
  std::shared_ptr<IComponent> GetScript(std::string_view key);

private:
  std::map<std::string, std::function<std::shared_ptr<IComponent>()>,
           std::less<>>
      map_comp;
};

} // namespace Main
} // namespace Engine
