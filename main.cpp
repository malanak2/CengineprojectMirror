#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>

#include "Editor/Components/Registrar.hpp"
#include "Editor/ImGuiRegistrar.hpp"
#include "Editor/ImGuiRenderers.hpp"
#include "Engine/Engine.hpp"

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
  ZoneScopedNS("main", 64);
  /// Inject functions
  {
    ZoneScopedN("Register ImGui") Editor::ImGuiR::Register();
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
