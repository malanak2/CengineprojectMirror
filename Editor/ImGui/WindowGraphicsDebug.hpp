#pragma once

#include "Graphics/Graphics.hpp"
#include "ImGui/Window.hpp"
#include <imgui.h>
namespace Editor {
class WindowGraphicsDebug : public Window {
public:
  std::string GetName() override { return "WindowGraphicsDebug"; }

protected:
  void draw() override {
    ImGui::Begin(GetName().c_str());
    if (ImGui::CollapsingHeader(
            (std::format("Materials ({})",
                         Engine::Graphics::Main::materials.size())
                 .c_str()))) {
      for (const auto &m : Engine::Graphics::Main::materials) {

        if (ImGui::CollapsingHeader(
                std::format("{} ({})", m.first,
                            m.second->renderableObjects.size())
                    .c_str())) {
          for (const auto &o : m.second->renderableObjects) {
            ImGui::Text("%s", o->object.lock()->_name.c_str());
          }
        }
      }
    }
    if (ImGui::CollapsingHeader(
            std::format("Textures ({})",
                        Engine::Graphics::Main::textures.size())
                .c_str())) {
      for (const auto &[path, tex] : Engine::Graphics::Main::textures) {
        ImGui::Text("%s", path.c_str());
      }
    }
    ImGui::End();
  }
};
} // namespace Editor
