#include "ScriptSystem.hpp"
#include "EngineScripts.hpp"

namespace Engine::Main {
  void RegisterEngineScripts() {
    ScriptSystem::instance->scan_scope<kEngineScope>();
  }
}