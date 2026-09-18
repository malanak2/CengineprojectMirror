#pragma once
#include "Components/ComponentScript.hpp"
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
    auto engineInstance = Engine::Main::instance;                              \
    (engineInstance->scriptSystem->RegisterScript(script))                     \
  }

//!
//! @brief System for registering user scripts. Uses the std::type_index
//!
class ScriptSystem {
public:
  bool RegisterScript(std::shared_ptr<ComponentScript> script);
  std::shared_ptr<ComponentScript> GetScript(std::string_view key);

private:
  std::map<std::string_view, std::shared_ptr<ComponentScript>> map_comp;
};

} // namespace Main
} // namespace Engine
