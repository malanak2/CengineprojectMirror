#include "IUniform.hpp"
#include <imgui.h>
#include <memory>
#include <string>
using namespace Engine::Graphics;
void IUniform::RenderImGui() {
  ImGui::Text("Please implement %s", __PRETTY_FUNCTION__);
}

IUniform::IUniform(UniformType t, unsigned int id, unsigned int offset,
                   std::shared_ptr<std::string> name) {
  type = t;
  info.offset = offset;
  info.id = id;
  info.name = name;
}
