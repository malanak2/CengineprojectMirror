#include "IUniform.hpp"
#include "Util/LoggerUtil.hpp"
#include <imgui.h>
#include <memory>
#include <string>
using namespace Engine::Graphics;
void IUniform::RenderImGui() {
  ImGui::Text("Please implement %s", __PRETTY_FUNCTION__);
}
json Engine::Graphics::IUniform::ToJson() const {
  SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Please implement {}",
                     __PRETTY_FUNCTION__);
  return nlohmann::json("{\"PLEASE IMPLEMENT THIS}\":\"h\"}");
}
void Engine::Graphics::IUniform::FromJson(json) const {
  SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Please implement {}",
                     __PRETTY_FUNCTION__);
}

IUniform::IUniform(UniformType t, unsigned int id, unsigned int offset,
                   std::shared_ptr<std::string> name) {
  type = t;
  info.offset = offset;
  info.id = id;
  info.name = name;
}
