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
class WindowScene : public Window {
public:
  std::shared_ptr<Engine::SceneObject> sceneObject;
  std::string GetName() override { return "Scene"; }

protected:
  std::shared_ptr<Engine::SceneObject> newObjectParent;

  bool wasSavePressedThisFrame;
  char namebuf[64];
  char matbuf[64];
  float coords[3];
  float rotation[4];
  void draw() override {
    ZoneScoped;
    auto scene = Engine::Engine::instance->current_scene;
    ImGui::Begin("Scene");
    ImGui::Text("Edit current scene");
    IMGUI_CHECKBOX("Wireframe mode", false, [](bool state) {
      if (state) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      }
    });
    IMGUI_CHECKBOX("AntiAliasing", Config::inst->graphics->enableAntiAliasing,
                   [](bool state) {
                     if (state) {
                       glEnable(GL_MULTISAMPLE);
                     } else {
                       glDisable(GL_MULTISAMPLE);
                     }
                   });
    IMGUI_CHECKBOX("Vsync", Config::inst->graphics->enableVsync,
                   [](bool state) {
                     if (state) {
                       glfwSwapInterval(1);
                     } else {
                       glfwSwapInterval(0);
                     }
                   });
    if (ImGui::CollapsingHeader("Objects")) {
      ShowSceneObjectMenu(&scene->objects);
    }
    if (sceneObject != nullptr) {
      ImGui::Text("Scene object %s", sceneObject->instance->_name.c_str());
      if (ImGui::Button("Deselect"))
        sceneObject = nullptr;
      else {
        ImGui::Text("Parent: %s",
                    sceneObject->Parent.lock()
                        ? sceneObject->Parent.lock()->instance->_name.c_str()
                        : "nullptr");
        if (ImGui::BeginDragDropTarget()) {
          if (const ImGuiPayload *payload =
                  ImGui::AcceptDragDropPayload("OBJ_PARENT")) {
            Engine::SceneObject *draggedObj =
                *(Engine::SceneObject **)payload->Data;
            if (draggedObj->shared_from_this() != sceneObject) {
              sceneObject->SetParent(draggedObj->shared_from_this());
            }
          }
          ImGui::EndDragDropTarget();
        }
      }
    } else {
      ImGui::Text("New Scene Object");
      ImGui::InputText("Name", namebuf, 64);
      ImGui::InputText("Material path", matbuf, 64);
      ImGui::InputFloat3("Position: ", coords);
      ImGui::InputFloat4("Rotation: ", coords);
      ImGui::Text("Parent: %s", newObjectParent
                                    ? newObjectParent->instance->_name.c_str()
                                    : "nullptr");
      if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload *payload =
                ImGui::AcceptDragDropPayload("OBJ_PARENT")) {
          Engine::SceneObject *draggedObj =
              *(Engine::SceneObject **)payload->Data;
          newObjectParent = draggedObj->shared_from_this();
        }
        ImGui::EndDragDropTarget();
      }
      if (newObjectParent != nullptr) {
        ImGui::SameLine();
        if (ImGui::Button("Remove")) {
          newObjectParent = nullptr;
        }
      }
      if (ImGui::Button("Create")) {
        auto o = std::make_shared<Engine::Object>(scene);
        std::vector<float> pos = {};
        std::vector<float> rot = {};
        pos.assign(coords, coords + sizeof(coords) / sizeof(float));
        rot.assign(rotation, rotation + sizeof(rotation) / sizeof(float));
        o->fromParams(namebuf, {}, pos, rot);
        scene->Instantiate(o, newObjectParent);
      }
    }
    ImGui::Text("File scene controls");
    ImGui::InputText("Scene location", &scene->path[0], 100);

    if (ImGui::Button("Save scene")) {
      if (!wasSavePressedThisFrame) {
        wasSavePressedThisFrame = true;
        SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Saving scene to {}...",
                           scene->path);
        json scenejs = scene->ToJson();
        auto st = scenejs.dump();
        FileUtil::SaveFile(scene->path, &st);
      }
    } else {
      wasSavePressedThisFrame = false;
    }

    ImGui::End();
  }
  void ShowSceneObjectMenu(
      std::vector<std::shared_ptr<Engine::SceneObject>> *sceneObjects) {
    ZoneScoped;
    if (!sceneObjects)
      return;
    for (auto &obj : *sceneObjects) {
      if (!obj || !obj->instance)
        continue;
      ImGui::PushID(obj.get());

      ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                 ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                 ImGuiTreeNodeFlags_SpanAvailWidth |
                                 ImGuiTreeNodeFlags_AllowOverlap;
      if (obj->Children.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
      }

      bool open = ImGui::TreeNodeEx(obj->instance->_name.c_str(), flags);

      if (ImGui::BeginDragDropSource()) {
        Engine::SceneObject *ptr = obj.get();
        ImGui::SetDragDropPayload("OBJ_PARENT", &ptr,
                                  sizeof(Engine::SceneObject *));
        ImGui::Text("%s", obj->instance->_name.c_str());
        ImGui::EndDragDropSource();
      }

      if (sceneObject != obj) {
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 50);
        if (ImGui::Button("Select")) {
          sceneObject = obj;
        }
      }

      if (open) {
        ShowSceneObjectMenu(&obj->Children);
        ImGui::TreePop();
      }

      ImGui::PopID();
    }
  }
};
} // namespace Editor
