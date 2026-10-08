#pragma once

#include "Config/Config.hpp"
#include "Engine.hpp"
#include "ImGuiMacros.hpp"
#include "InputSystem.hpp"
#include "Util/FileUtil.hpp"
#include "Window.hpp"
#include <imgui.h>
#include <tracy/Tracy.hpp>
namespace Editor {
class WindowInputMaps : public Window {
public:
  std::string GetName() override { return "Input Maps"; }

protected:
  void draw() override {
    ImGui::Begin("Input Maps");
    RenderInputMap("Press",
                   &(Engine::InputSystem::instance->keybind_map_press));
    RenderInputMap("Hold", &(Engine::InputSystem::instance->keybind_map_hold));
    RenderInputMap("Hold Text",
                   &(Engine::InputSystem::instance->keybind_map_hold_text));
    RenderInputMap("Release",
                   &(Engine::InputSystem::instance->keybind_map_release));
    ImGui::End();
  }
  void RenderInputMap(
      std::string name,
      std::map<int, std::vector<std::shared_ptr<Engine::Keybind>>> *map) {
    if (ImGui::CollapsingHeader(("KeyMap: " + name).c_str())) {
      for (auto const &[scancode, kvec] : *map) {
        if (kvec.size() == 0)
          continue;
        ImGui::Indent(10);
        if (ImGui::TreeNode(glfwGetKeyName(kvec[0]->key, scancode))) {
          for (auto keybind : kvec) {
            ImGui::Text(
                "ID: %s, Default key: %s", keybind->name.c_str(),
                glfwGetKeyName(keybind->default_key,
                               glfwGetKeyScancode(keybind->default_key)));
            if (ImGui::TreeNode(("Mods##" + keybind->name).c_str())) {
              bool shift = (keybind->mods & GLFW_MOD_SHIFT) != 0;
              if (ImGui::Checkbox(("Shift##" + keybind->name).c_str(),
                                  &shift)) {
                if (shift) {
                  keybind->mods |= GLFW_MOD_SHIFT;
                } else {
                  keybind->mods &= ~GLFW_MOD_SHIFT;
                }
              }
              bool ctrl = (keybind->mods & GLFW_MOD_CONTROL) != 0;
              if (ImGui::Checkbox(("Ctrl##" + keybind->name).c_str(), &ctrl)) {
                if (ctrl) {
                  keybind->mods |= GLFW_MOD_CONTROL;
                } else {
                  keybind->mods &= ~GLFW_MOD_CONTROL;
                }
              }
              bool alt = (keybind->mods & GLFW_MOD_ALT) != 0;
              if (ImGui::Checkbox(("Alt##" + keybind->name).c_str(), &alt)) {
                if (alt) {
                  keybind->mods |= GLFW_MOD_ALT;
                } else {
                  keybind->mods &= ~GLFW_MOD_ALT;
                }
              }
              ImGui::TreePop();
            }
            ImGui::Checkbox(("Enabled##" + keybind->name).c_str(),
                            &keybind->isEnabled);
          }
          ImGui::TreePop();
        }
        ImGui::Unindent(10);
      }
    }

    ImGui::Separator();
  }
};
} // namespace Editor
