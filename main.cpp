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

void sigsegvHandler(int sig) {
  SPDLOG_LOGGER_CRITICAL(ENGINE_UTIL_LOGGER,
                         "Program has crashed. Stacktrace: {}",
                         std::to_string(std::stacktrace::current()));
  exit(sig);
}

void sigabrtHandler(int sig) { sigsegvHandler(sig); }
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
  signal(SIGSEGV, sigsegvHandler);
  signal(SIGABRT, sigabrtHandler);
  try {
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
  } catch (const std::exception &e) {
    SPDLOG_LOGGER_CRITICAL(ENGINE_UTIL_LOGGER,
                           "Program has crashed. Stacktrace: {}",
                           std::to_string(std::stacktrace::current()));
  }
  return 0;
}
