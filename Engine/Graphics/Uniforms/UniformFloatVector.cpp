#include "UniformFloatVector.hpp"
#include "Graphics/Graphics.hpp"
#include "Graphics/Texture.hpp"
#include "Util/LoggerUtil.hpp"
#include <imgui.h>
using namespace Engine::Graphics;
void UniformFloatVector::Use(unsigned int offset) {
  CHECK_GL_ERROR();
  glBindBuffer(GL_UNIFORM_BUFFER, info.id);
  if (data.size() != 4) {
    SPDLOG_LOGGER_WARN(
        ENGINE_UTIL_LOGGER,
        "Uniform {} has a bad size, please file an issue for me to implement",
        info.name ? *info.name : "");
  }
  float data_[4] = {0.0f, 0.0f, 0.0f, 1.0f};
  for (size_t i = 0; i < data.size() && i < 4; ++i) {
    data_[i] = data[i];
  }
  glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(data_), data_);
  CHECK_GL_ERROR();
}
void UniformFloatVector::RenderImGui() {
  std::string key = *info.name;
  switch (data.size()) {
  case 1: {
    ImGui::InputFloat(&key[0], &data[0]);
    break;
  }
  case 2: {
    ImGui::InputFloat2(&key[0], &data[0]);
    break;
  }
  case 3: {
    ImGui::InputFloat3(&key[0], &data[0]);
    break;
  }
  case 4: {
    if (key == "color") {
      ImGui::ColorPicker4(&key[0], &data[0]);
    } else {
      ImGui::InputFloat4(&key[0], &data[0]);
    }
    break;
  }
  default: {
    ImGui::Text("Unsupported float uniform %s of length %zu", &key[0],
                data.size());
    break;
  }
  }
}
Engine::Graphics::UniformFloatVector::UniformFloatVector(
    UniformType t, unsigned int id, unsigned int offset,
    std::shared_ptr<std::string> name, std::vector<float> data)
    : IUniform(t, id, offset, name) {
  this->data = data;
}

json UniformFloatVector::ToJson() const {
  json j = IUniform::ToJson();
  j["data"] = data;
  return j;
}
