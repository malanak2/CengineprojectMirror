#pragma once

#include "ImGui/WindowChangeKeybind.hpp"
#include "InputSystem.hpp"
#include "Window.hpp"
#include <imgui.h>
#include <tracy/Tracy.hpp>
namespace Editor {
class WindowInputMaps : public Window {
public:
  std::string GetName() override { return "Input Maps"; }
private:
  static std::string KeyLabel(int key, int scancode) {
    if (const char *n = glfwGetKeyName(key, scancode))
      return n;
    switch (key) {
    case GLFW_KEY_SPACE:         return "Space";
    case GLFW_KEY_ESCAPE:        return "Escape";
    case GLFW_KEY_ENTER:         return "Enter";
    case GLFW_KEY_TAB:           return "Tab";
    case GLFW_KEY_BACKSPACE:     return "Backspace";
    case GLFW_KEY_LEFT:          return "Left";
    case GLFW_KEY_RIGHT:         return "Right";
    case GLFW_KEY_UP:            return "Up";
    case GLFW_KEY_DOWN:          return "Down";
    case GLFW_KEY_LEFT_SHIFT:    return "Left Shift";
    case GLFW_KEY_RIGHT_SHIFT:   return "Right Shift";
    case GLFW_KEY_LEFT_CONTROL:  return "Left Ctrl";
    case GLFW_KEY_RIGHT_CONTROL: return "Right Ctrl";
    case GLFW_KEY_LEFT_ALT:      return "Left Alt";
    case GLFW_KEY_RIGHT_ALT:     return "Right Alt";
    default:                     return "Key " + std::to_string(key);
    }
  }
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
    RenderMouseInputMap(
        "Mouse: Move",
        &(Engine::InputSystem::instance->keybind_vector_mouse_move));
    RenderMouseInputMap(
        "Mouse: Click",
        &(Engine::InputSystem::instance->keybind_vector_mouse_click));
    RenderMouseInputMap(
        "Mouse: Release",
        &(Engine::InputSystem::instance->keybind_vector_mouse_release));
    RenderMouseInputMap(
        "Mouse: Scroll",
        &(Engine::InputSystem::instance->keybind_vector_mouse_scroll));
    ImGui::End();
  }
  void RenderInputMap(
      const std::string &name,
      std::map<int, std::vector<std::shared_ptr<Engine::Keybind>>> *map) {
    if (ImGui::CollapsingHeader(("KeyMap: " + name).c_str())) {
      for (auto const &[scancode, kvec] : *map) {
        if (kvec.size() == 0)
          continue;
        ImGui::Indent(10);
        if (ImGui::TreeNode(KeyLabel(kvec[0]->key, scancode).c_str())) {
          for (auto keybind : kvec) {
            ImGui::Text(
                "ID: %s, Default key: %s", keybind->name.c_str(),
                glfwGetKeyName(keybind->default_key,
                               glfwGetKeyScancode(keybind->default_key)));
            if (ImGui::Button(("Change##" + keybind->name).c_str())) {
              Editor::WindowChangeKeybind::OpenFor(keybind);
            }
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

  void
  RenderMouseInputMap(std::string name,
                      std::vector<std::shared_ptr<Engine::KeybindMouse>> *vec) {
    if (ImGui::CollapsingHeader(name.c_str())) {
      ImGui::Indent(10);
      for (auto &keybind : *vec) {
        if (!keybind)
          continue;
        ImGui::Text("ID: %s, Button: %s", keybind->name.c_str(),
                    Engine::GetGlfwMouseButtonName(keybind->button).c_str());
        if (ImGui::TreeNode(("Mods##" + keybind->name).c_str())) {
          bool shift = (keybind->mods & GLFW_MOD_SHIFT) != 0;
          if (ImGui::Checkbox(("Shift##" + keybind->name).c_str(), &shift)) {
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
      ImGui::Unindent(10);
      ImGui::Separator();
    }
  }
};
} // namespace Editor
