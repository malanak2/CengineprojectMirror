#pragma once

#include "ImGui/Window.hpp"
#include "ImGuiMacros.hpp"
#include <imgui.h>
#include <memory>
#include <vector>

namespace Editor {
class WindowToggles : public Window {
public:
  std::string GetName() override { return "WindowToggles"; }

  std::vector<std::shared_ptr<Window>> *windows;

protected:
  void draw() override {
    ImGui::Begin("WindowToggles");
    for (auto w : *windows) {
      ImGui ::Checkbox(w->GetName().c_str(), &w->isOpen);
    }
    ImGui::End();
  }
};
} // namespace Editor
