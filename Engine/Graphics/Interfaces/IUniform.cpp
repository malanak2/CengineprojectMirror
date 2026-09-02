#include "IUniform.hpp"
#include "Graphics/Uniforms/UniformFloatVector.hpp"
#include "Util/LoggerUtil.hpp"
#include <imgui.h>
#include <memory>
#include <string>
using namespace Engine::Graphics;
void IUniform::RenderImGui() {
  ImGui::Text("Please implement %s", __PRETTY_FUNCTION__);
}
json Engine::Graphics::IUniform::ToJson() const {
  json j;
  j["type"] = type;
  j["id"] = info.id;
  j["offset"] = info.offset;
  if (info.name) {
    j["name"] = *info.name;
  }
  return j;
}
void Engine::Graphics::IUniform::FromJson(json) const {
  SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Please implement {}",
                     __PRETTY_FUNCTION__);
}

std::shared_ptr<IUniform> IUniform::CreateUniformFromJson(const json &j) {
  if (j.is_array()) {
    std::vector<float> data = j.get<std::vector<float>>();
    return std::make_shared<UniformFloatVector>(
        UniformType::Vector, 0u, 0u, std::make_shared<std::string>(""), data);
  }

  if (!j.is_object()) {
    return nullptr;
  }

  UniformType type = UniformType::Vector;
  if (j.contains("type")) {
    if (j["type"].is_string()) {
      std::string t = j["type"].get<std::string>();
      if (t == "vector" || t == "Vector") {
        type = UniformType::Vector;
      } else if (t == "sampler" || t == "Sampler") {
        type = UniformType::Sampler;
      }
    } else if (j["type"].is_number_integer()) {
      type = static_cast<UniformType>(j["type"].get<int>());
    }
  }

  switch (type) {
  case UniformType::Vector: {
    unsigned int id = j.value("id", 0u);
    unsigned int offset = j.value("offset", 0u);
    std::string name = j.value("name", "");
    std::vector<float> data = j.value("data", std::vector<float>{});

    return std::make_shared<UniformFloatVector>(
        type, id, offset, std::make_shared<std::string>(name), data);
  }
  case UniformType::Sampler: {
    break;
  }
  default: {
    break;
  }
  }
  return nullptr;
}

IUniform::IUniform(UniformType t, unsigned int id, unsigned int offset,
                   std::shared_ptr<std::string> name) {
  type = t;
  info.offset = offset;
  info.id = id;
  info.name = name;
}
