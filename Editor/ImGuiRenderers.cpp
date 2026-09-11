#include "ImGuiRenderers.hpp"
#include "Graphics/Uniforms/UniformFloatVector.hpp"
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

void ImGuiUniformRenderer::Init() {
  IMGUI_REGISTER_UNIFORM(Engine::Graphics::UniformFloatVector, {
    auto uniformCast =
        std::static_pointer_cast<Engine::Graphics::UniformFloatVector>(uniform);
    std::string key = *uniformCast->info.name;
    switch (uniformCast->data.size()) {
    case 1: {
      ImGui::InputFloat(&key[0], &uniformCast->data[0]);
      break;
    }
    case 2: {
      ImGui::InputFloat2(&key[0], &uniformCast->data[0]);
      break;
    }
    case 3: {
      ImGui::InputFloat3(&key[0], &uniformCast->data[0]);
      break;
    }
    case 4: {
      if (key == "color") {
        ImGui::ColorPicker4(&key[0], &uniformCast->data[0]);
      } else {
        ImGui::InputFloat4(&key[0], &uniformCast->data[0]);
      }
      break;
    }
    default: {
      ImGui::Text("Unsupported float uniform %s of length %zu", &key[0],
                  uniformCast->data.size());
      break;
    }
    }
  });
}
} // namespace Editor::ImGuiRenderer
