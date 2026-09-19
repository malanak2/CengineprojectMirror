#include <csignal>
#include <exception>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <stacktrace>
#include <tracy/Tracy.hpp>

#include "Editor/ImGuiRegistrar.hpp"
#include "Editor/ImGuiRenderers.hpp"
#include "Engine/Engine.hpp"
#include "Graphics/Graphics.hpp"
#include "Graphics/Texture.hpp"
#include "ScriptSystem.hpp"
#include "Util/LoggerUtil.hpp"

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
int main() {
  ZoneScopedNS("main", 64);
  /// Inject functions
  /// Init engine
  Editor::ImGuiR::Register();
  Engine::Engine::Init();
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
