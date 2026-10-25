#pragma once

#include "Config/Config.hpp"
#include "Engine.hpp"
#include "ImGuiMacros.hpp"
#include "Object.hpp"
#include "Scene.hpp"
#include "Util/FileUtil.hpp"
#include "Window.hpp"
#include <imgui.h>
#include <memory>
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
    if (ImGui::CollapsingHeader("Controls")) {
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
      ImGui::Checkbox("Render bounding boxes",
                      &Config::inst->graphics->renderBoundingBox);
      ImGui::Checkbox("Enable simulation",
                      &Engine::Engine::isSimulationEnabled);
      if (ImGui::BeginCombo(
              "Scene camera",
              Engine::Engine::instance->current_scene->camera->object.lock()
                  ? Engine::Engine::instance->current_scene->camera->object
                        .lock()
                        ->_name.c_str()
                  : "nullptr")) {
        for (auto &cam : Engine::Engine::instance->current_scene->cameras) {
          bool is_selected =
              (Engine::Engine::instance->current_scene->camera == cam);
          if (ImGui::Selectable(cam->object.lock()
                                    ? cam->object.lock()->_name.c_str()
                                    : "nullptr",
                                is_selected)) {
            Engine::Engine::instance->current_scene->camera = cam;
          }
          if (is_selected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      if (ImGui::BeginCombo("Default camera",
                            Engine::Engine::instance->current_scene
                                ->default_camera.c_str())) {
        for (auto &cam : Engine::Engine::instance->current_scene->cameras) {
          bool is_selected =
              (Engine::Engine::instance->current_scene->camera == cam);
          if (ImGui::Selectable(cam->object.lock()
                                    ? cam->object.lock()->_name.c_str()
                                    : "nullptr",
                                is_selected)) {
            Engine::Engine::instance->current_scene->default_camera =
                cam->object.lock() ? cam->object.lock()->_name : "nullptr";
          }
          if (is_selected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
    }
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
      ContextMenu(obj->instance);

      if (ImGui::BeginDragDropSource()) {
        Engine::SceneObject *ptr = obj.get();
        ImGui::SetDragDropPayload("OBJ_PARENT", &ptr,
                                  sizeof(Engine::SceneObject *));
        ImGui::Text("%s%s", obj->instance->_name.c_str(),
                    obj->instance->IsValid() ? "" : " ERROR");
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
  void ContextMenu(const std::shared_ptr<Engine::Object> &obj) {
    if (ImGui::BeginPopupContextItem(("ContextMenu##" + obj->_name).c_str())) {
      if (ImGui::MenuItem(("New Object##" + obj->_name).c_str())) {
        auto o = std::make_shared<Engine::Object>();
        Engine::Engine::instance->current_scene->Instantiate(
            o, obj->sceneobject.lock());
      }
      if (ImGui::MenuItem(("Delete##" + obj->_name).c_str())) {
        Engine::Engine::instance->current_scene->RemoveObject(
            obj->sceneobject.lock());
      }
      if (ImGui::MenuItem(("Delete Cascade##" + obj->_name).c_str())) {
        Engine::Engine::instance->current_scene->RemoveObjectCascade(
            obj->sceneobject.lock());
      }
      ImGui::EndPopup();
    }
  }
};
} // namespace Editor
