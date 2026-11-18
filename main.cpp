#include <iostream>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>

#include "Testing/Testing.hpp"

#include "Editor/ImGuiRegistrar.hpp"
#include "Editor/ImGuiRenderers.hpp"
#include "Engine/Engine.hpp"
#ifdef WIN32
#include "out/Generated/AllComponents.hpp"
#else
#include "out/Debug/Generated/AllComponents.hpp"
#endif


// No need to make allocations synchronous without Tracy enabled
#ifdef TRACY_ENABLE
std ::mutex memoryLock;
void *operator new(std ::size_t count) {
  std ::lock_guard lock(memoryLock);
  auto ptr = malloc(count);
  TracyAllocS(ptr, count,64);
  return ptr;
}
void operator delete(void *ptr) noexcept {
  std ::lock_guard lock(memoryLock);
  TracyFreeS(ptr, 64);
  free(ptr);
}
#endif

int main() {
  Editor::ImGuiR::ImGuiRegistrar rg;
  ZoneScopedNS("main", 64);
  {
    ZoneScopedN("Create script registry")
    Engine::Main::ScriptSystem::instance->create_global_registry<^^::>();
    ReflectionRegistrar::Get().RegisterReflectedScope<^^::>();
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
  {
    Editor::Testing::Setup();
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
