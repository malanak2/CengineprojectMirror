#pragma once

#include "ImGui/Window.hpp"
#include "ImGui/WindowScene.hpp"
#include "ImGuiRenderers.hpp"
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <imgui.h>
#include <tracy/Tracy.hpp>
namespace Editor {
class WindowObjectInspector : public Window {
public:
  WindowObjectInspector(std::shared_ptr<WindowScene> sceneWindow) {
    this->sceneWindow = sceneWindow;
  }
  std::string GetName() override { return "Object Inspector"; }

protected:
  std::shared_ptr<WindowScene> sceneWindow;
  // https://en.cppreference.com/cpp/string/byte/tolower
  std::string str_tolower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
  }
  void draw() override {
    ZoneScoped;
    auto sceneObject = sceneWindow->sceneObject;
    ImGui::Begin("Object inspetor");
    if (sceneObject == nullptr) {
      ImGui::Text("Please select an object");
      ImGui::End();
      return;
    }
    char buf[64] = {0};
    strncpy(buf, sceneObject->instance->_name.c_str(), 63);

    if (ImGui::InputText("Name:", buf, 64)) {
      sceneObject->instance->_name = buf; // instance correctly updates the size
    }
    if (ImGui::CollapsingHeader("Values")) {
      auto obj = sceneObject->instance;
      if (obj) {
        ImGui::InputFloat3("XYZ", &obj->_position[0]);
        ImGui::InputFloat("Scale", &obj->scale);

        static glm::vec3 currentAngles =
            glm::degrees(glm::eulerAngles(obj->_rotation));
        glm::vec3 newAngles = currentAngles;

        if (ImGui::DragFloat3("Rotation", &newAngles[0], 1.0f)) {
          glm::vec3 delta = newAngles - currentAngles;

          if (delta.x != 0.0f) {
            obj->_rotation =
                glm::angleAxis(glm::radians(delta.x), glm::vec3(1, 0, 0)) *
                obj->_rotation;
          }
          if (delta.y != 0.0f) {
            obj->_rotation =
                glm::angleAxis(glm::radians(delta.y), glm::vec3(0, 1, 0)) *
                obj->_rotation;
          }
          if (delta.z != 0.0f) {
            obj->_rotation =
                glm::angleAxis(glm::radians(delta.z), glm::vec3(0, 0, 1)) *
                obj->_rotation;
          }

          obj->_rotation = glm::normalize(obj->_rotation);
          currentAngles = newAngles;
        }
      }
    }
    std::vector<std::string> names = std::vector<std::string>();
    for (auto [type, comp] : sceneObject->instance->_components) {
      if (comp != nullptr)
        if (ImGui::CollapsingHeader(comp->GetName().data())) {
          ImGuiR::ImGuiComponentRenderer::instance->Render(comp);
        }
      ImGui::Separator();
      ImGui::Separator();
      names.insert(names.end(), std::string(comp->GetName()));
    }
    ImGui::Text("Add components");
    static char search[256] = "";
    ImGui::InputText("Title", search, sizeof(search));
    for (const auto &[name, factory] :
         Engine::Main::ScriptSystem::instance->map_comp) {
      auto it = find(names.begin(), names.end(), name);

      if (it != names.end()) {
        continue;
      }
      if (search[0] != '\0') {

        if (str_tolower(name).find(str_tolower(search)) == std::string::npos) {
          continue;
        }
      }
      if (ImGui::Button(name.c_str())) {
        TracyMessage(("Added component " + name).c_str(),
                     ("Added component " + name).size());
        sceneObject->instance->AddComponent(name);
      }
    }
    ImGui::End();
  }
};
} // namespace Editor
