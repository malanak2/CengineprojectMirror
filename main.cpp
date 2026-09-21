#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>

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
  Editor::ImGuiR::Register();
  /// Init engine
  Engine::Engine::Init();
  // Init editor ImGui
  Editor::ImGuiR::ImGuiUniformRenderer::Init();
  Editor::ImGuiR::ImGuiComponentRenderer::Init();
  /// Register components
  // REGISTER_SCRIPT(script);
  /// Load scene
  Engine::Engine::LoadScene();
  /// Run
  Engine::Engine::instance->Run();
  return 0;
}
