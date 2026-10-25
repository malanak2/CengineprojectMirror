#pragma once

#include "ImGui/Window.hpp"
#include "ImGui/WindowScene.hpp"
#include "ImGuiRenderers.hpp"
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <imgui.h>
#include <tracy/Tracy.hpp>
#include <utility>
namespace Editor {
class WindowObjectInspector : public Window {
public:
  WindowObjectInspector(std::shared_ptr<WindowScene> sceneWindow) {
    this->sceneWindow = std::move(sceneWindow);
  }
  std::string GetName() override { return "Object Inspector"; }

protected:
  std::shared_ptr<WindowScene> sceneWindow;
  // https://en.cppreference.com/cpp/string/byte/tolower
  static std::string str_tolower(std::string s) {
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
    strncpy_s(buf, sceneObject->instance->_name.c_str(), 63);

    if (ImGui::InputText("Name:", buf, 64)) {
      sceneObject->instance->_name = buf; // instance correctly updates the size
    }
    if (ImGui::CollapsingHeader("Values")) {
      auto obj = sceneObject->instance;
      if (obj) {
        glm::vec3 localPos = obj->GetLocalPosition();
        if (ImGui::InputFloat3("XYZ", &localPos[0])) {
          obj->SetPosition(localPos);
        }
        ImGui::InputFloat3("Scale", &obj->scale[0]);
        static glm::vec3 angles = glm::vec3(0.0f);
        static Engine::Object *lastObject = nullptr;

        glm::quat local = obj->GetLocalRotation();
        glm::quat fromAngles = glm::quat(glm::radians(angles));
        // q and -q are the same rotation, so compare |dot|
        bool sameRotation = std::abs(glm::dot(local, fromAngles)) > 0.999999f;

        if (lastObject != obj.get() || !sameRotation) {
          angles = glm::degrees(glm::eulerAngles(local));
          lastObject = obj.get();
        }

        if (ImGui::DragFloat3("Rotation", &angles[0], 1.0f)) {
          obj->SetRotation(glm::normalize(glm::quat(glm::radians(angles))));
        }
      }
    }
    std::vector<std::string> names = std::vector<std::string>();
    std::string todel;
    for (auto &[type, comp] : sceneObject->instance->_components) {
      if (comp != nullptr) {
        bool isCOpen;
        if (comp->IsValid()) {
          isCOpen = ImGui::CollapsingHeader(comp->GetName().data());
        } else {
          isCOpen = ImGui::CollapsingHeader(
              ("ERROR " + (std::string)comp->GetName().data() + " ERROR")
                  .c_str());
        }
        if (isCOpen) {
          ImGuiR::ImGuiComponentRenderer::instance->Render(comp);
          if (ImGui::Button(
                  ("Remove##" + (std::string)comp->GetName().data()).c_str())) {
            todel = comp->GetName().data();
          }
        }
      }
      ImGui::Separator();
      ImGui::Separator();
      names.insert(names.end(), std::string(comp->GetName()));
    }
    if (todel != "") {
      sceneObject->instance->RemoveComponent(todel);
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
