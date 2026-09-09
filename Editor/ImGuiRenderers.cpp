#include "ImGuiRenderers.hpp"
#include "Util/LoggerUtil.hpp"
#include <imgui.h>
#include <memory>
#include <spdlog/spdlog.h>
#include <typeindex>

namespace Editor::ImGuiRenderer {

std::shared_ptr<ImGuiUniformRenderer> ImGuiUniformRenderer::instance =
    std::make_shared<ImGuiUniformRenderer>();

void ImGuiUniformRenderer::Register(
    std::type_index type,
    void (*func)(std::shared_ptr<Engine::Graphics::IUniform>)) {
  if (funcs.contains(type)) {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Tried to re-register type");
    return;
  }
  funcs.insert({type, func});
}
void ImGuiUniformRenderer::Render(
    std::shared_ptr<Engine::Graphics::IUniform> uni) {
  auto t = std::type_index(typeid(*uni));
  if (funcs.contains(t)) {
    funcs[t](uni);
  } else {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Typeid {} not registered",
                        t.name());
  }
}

} // namespace Editor::ImGuiRenderer
