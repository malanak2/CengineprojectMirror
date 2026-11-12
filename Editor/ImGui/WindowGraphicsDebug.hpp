#pragma once

#include "Graphics/Graphics.hpp"
#include "ImGui/Window.hpp"
#include <imgui.h>

#include "Graphics/Components/ComponentAnimator.hpp"

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
      for (const auto& path : Engine::Graphics::Main::textures | std::views::keys) {
        ImGui::Text("%s", path.c_str());
      }
    }
    if (ImGui::CollapsingHeader(
            std::format("Animations ({})",
                        Engine::Graphics::ComponentAnimator::cache.size())
                .c_str())) {
      for (const auto& [path, can] : Engine::Graphics::ComponentAnimator::cache) {
        if (ImGui::CollapsingHeader(std::format("{}", path.c_str()).c_str()))
        {
          ImGui::Text("Anim count: %d", can.animations.size());
          ImGui::Text("Default animation: %s", !can.current_animation.empty() ? can.current_animation.c_str() : "nullptr");
          ImGui::Text("Default state: %s", can.isPlaying ? "playing" : "stopped");
        }
      }
    }
    ImGui::End();
  }
};
} // namespace Editor
