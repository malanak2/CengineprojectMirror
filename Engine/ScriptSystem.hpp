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
#define REGISTER_SCRIPT(script)                                                \
  {                                                                            \
    auto engineInstance = Engine::Main::instance;                              \
  }

class ScriptSystem {
public:
  bool RegisterScript(std::shared_ptr<ComponentScript> script);
  std::shared_ptr<ComponentScript> GetScript(std::string key);

private:
  std::map<std::string, std::shared_ptr<ComponentScript>> map_comp;
};

} // namespace Main
} // namespace Engine
