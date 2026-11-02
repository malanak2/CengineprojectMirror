#include <iostream>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>

#include "Editor/Components/Registrar.hpp"
#include "Editor/Components/ComponentControllable.hpp"
#include "Editor/ImGuiRegistrar.hpp"
#include "Editor/ImGuiRenderers.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Components/InvalidComponent.hpp"
#include "Engine/Physics/Components/ComponentPhysics.hpp"
#include "Editor/Components/TestInputComponent.hpp"

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
  Engine::Main::ScriptSystem::instance->create_global_registry<^^::>();
  for (const auto& script_info : Engine::Main::ScriptSystem::instance->All()) {
    if (script_info == nullptr) continue;
    std::cout << "Found Script: " << std::string(script_info->name )
              << "\n";
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
  // Init editor ImGui
  {
    ZoneScopedN("Register imgui renderers");
    Editor::ImGuiR::ImGuiUniformRenderer::Init();
    Editor::ImGuiR::ImGuiComponentRenderer::Init();
  }
  /// Register components
  {
    ZoneScopedN("Register comonents");
    Editor::Registrar::RegisterComponents();
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
