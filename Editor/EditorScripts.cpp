#include "ReflectionRegistrar.hpp"
#include "ScriptSystem.hpp"
#include "EngineScripts.hpp" 
#include "EditorScripts.hpp"

namespace Editor {
  constexpr std::meta::info kEditorScope = ^^::Editor;

  void RegisterEditorScripts() {
    Engine::Main::ScriptSystem::instance->scan_scope<kEditorScope>();

    auto& reg = ReflectionRegistrar::Get();
    reg.RegisterReflectedScope<Engine::Main::kEngineScope>();
    reg.RegisterReflectedScope<kEditorScope>();
  }
}