#include <iostream>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>

#include "Editor/ImGuiRegistrar.hpp"
#include "Editor/ImGuiRenderers.hpp"
#include "Engine/Engine.hpp"
#include "out/Debug/Generated/AllComponents.hpp"
// No need to make allocations synchronous without Tracy enabled
#ifdef TRACY_ENABLE
std ::mutex memoryLock;
void *operator new(std ::size_t count) {
  std ::lock_guard lock(memoryLock);
  auto ptr = malloc(count);
  TracyAlloc(ptr, count);
  return ptr;
}
void operator delete(void *ptr) noexcept {
  std ::lock_guard lock(memoryLock);
  TracyFree(ptr);
  free(ptr);
}
#endif

int main() {
  Editor::ImGuiR::ImGuiRegistrar rg;
  ZoneScopedNS("main", 64);
  {
    ZoneScopedN("Create script registry")
    Engine::Main::ScriptSystem::instance->create_global_registry<^^::>();
  }
  /// Inject functions
  {
    ZoneScopedN("Register ImGui");
    rg.Register();
  }
  /// Init engine
  {
    ZoneScopedN("Initialize Engine");
    Engine::Engine::Init();
  }
  // Print found components
  {
    // No zonescoped - come on, if this takes more than few microseconds there is something REALLY wrong (or you have like 100k components ig)
#ifdef DEBUG
    for (const auto& script_info : Engine::Main::ScriptSystem::instance->All())
    {
      if (script_info == nullptr) continue;
      SPDLOG_LOGGER_DEBUG(ENGINE_UTIL_LOGGER, "Registered script: {}, display name: {}", std::string(script_info->name), script_info->displayname);
    }
#endif
  }
  // Init editor ImGui
  {
    ZoneScopedN("Register imgui renderers");
    Editor::ImGuiR::ImGuiUniformRenderer::Init();
    Editor::ImGuiR::ImGuiComponentRenderer::Init();
  }
  /// Load scene
  {
    ZoneScopedN("Load scene");
    Engine::Engine::LoadScene();
  }
  /// Run
  Engine::Engine::instance->Run();
  return 0;
}
