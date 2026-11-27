#pragma once

#include "Config/Config.hpp"
#include "Engine.hpp"
#include "ImGuiMacros.hpp"
#include "Object.hpp"
#include "Scene.hpp"
#include "Util/FileUtil.hpp"
#include "Window.hpp"
#include <algorithm>
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
  char nestedPathBuf[128] = {0};
  std::shared_ptr<Engine::Scene> pendingReload;
  bool openReloadPopup = false;
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
      auto activeCam = scene->camera;
      auto activeCamObj = activeCam ? activeCam->object.lock() : nullptr;
      if (ImGui::BeginCombo("Scene camera", activeCamObj ? activeCamObj->sceneobject.lock()->_name.c_str() : "nullptr")) {
        for (auto &cam : Engine::Engine::instance->current_scene->cameras) {
          bool is_selected =
              (Engine::Engine::instance->current_scene->camera == cam);
          if (ImGui::Selectable(cam->object.lock()
                                    ? cam->object.lock()->sceneobject.lock()->_name.c_str()
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
                                    ? cam->object.lock()->sceneobject.lock()->_name.c_str()
                                    : "nullptr",
                                is_selected)) {
            Engine::Engine::instance->current_scene->default_camera =
                cam->object.lock() ? cam->object.lock()->sceneobject.lock()->_name : "nullptr";
          }
          if (is_selected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
    }
    bool oOpen = ImGui::CollapsingHeader("Objects");
    
    if (ImGui::BeginPopupContextItem())
    {
      if (ImGui::MenuItem("New object"))
      {
        auto o = std::make_shared<Engine::Object>();
        Engine::Engine::instance->current_scene->Instantiate(
            o, Engine::Engine::instance->current_scene->SelfScene()); 
      }
      ImGui::EndPopup();
    }
    if (oOpen) {
      ShowSceneObjectMenu(&scene->Children);
    }
    if (sceneObject != nullptr) {
      ImGui::Text("Scene object %s", sceneObject->_name.c_str());
      if (ImGui::Button("Deselect"))
        sceneObject = nullptr;
      else {
        auto parentSo = sceneObject->Parent.lock();
        ImGui::Text("Parent: %s", parentSo && parentSo->instance ? parentSo->_name.c_str() : "none");
        if (ImGui::Button("Move to owning scene")) {
          sceneObject->SetParent(nullptr);
        }
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
    char pathbuf[256] = {0};
    strncpy(pathbuf, scene->path.c_str(), sizeof(pathbuf) - 1);
    if (ImGui::InputText("Scene location", pathbuf, sizeof(pathbuf))) {
      scene->path = pathbuf;
    }
    if (ImGui::Button("Save scene")) {
      if (!wasSavePressedThisFrame) {
        wasSavePressedThisFrame = true;
        scene->Save();
      }
    } else {
      wasSavePressedThisFrame = false;
    }
    
    if (ImGui::Button("Reload scene")) {
      pendingReload = scene;
      openReloadPopup = true;
    }
    if (openReloadPopup) {
      ImGui::OpenPopup("Reload scene?");
      openReloadPopup = false;
    }
    if (ImGui::BeginPopupModal("Reload scene?", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("Reload \"%s\" from %s?\nUnsaved changes to it will be lost.",
                  pendingReload && pendingReload->instance
                      ? pendingReload->_name.c_str()
                      : "?",
                  pendingReload ? pendingReload->path.c_str() : "?");
      if (ImGui::Button("Reload")) {
        if (pendingReload)
          pendingReload->Reload();
        sceneObject = nullptr;
        pendingReload = nullptr;
        ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button("Cancel")) {
        pendingReload = nullptr;
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
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

      const bool hasChildren = !obj->Children.empty();
      ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                 ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                 ImGuiTreeNodeFlags_SpanAvailWidth |
                                 ImGuiTreeNodeFlags_AllowOverlap |
                                 ImGuiTreeNodeFlags_NoTreePushOnOpen;
      if (!hasChildren) {
        flags |= ImGuiTreeNodeFlags_Leaf;
      }

      std::string label = obj->_name;
      if (auto nested = std::dynamic_pointer_cast<Engine::Scene>(obj))
        label += nested->loadFailed ? " [scene: MISSING]" : " [scene]";
      bool open = ImGui::TreeNodeEx(label.c_str(), flags);
      ContextMenu(obj->instance);

      if (ImGui::BeginDragDropSource()) {
        Engine::SceneObject *ptr = obj.get();
        ImGui::SetDragDropPayload("OBJ_PARENT", &ptr,
                                  sizeof(Engine::SceneObject *));
        ImGui::Text("%s%s", obj->_name.c_str(),
                    obj->instance->IsValid() ? "" : " ERROR");
        ImGui::EndDragDropSource();
      }

      if (sceneObject != obj) {
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 50);
        if (ImGui::Button(("Select##" + obj->_name).c_str())) {
          sceneObject = obj;
        }
      }

      if (hasChildren && open) {
        ImGui::TreePush(label.c_str());
        ShowSceneObjectMenu(&obj->Children);
        ImGui::TreePop();
      }

      ImGui::PopID();
    }
  }
  void ContextMenu(const std::shared_ptr<Engine::Object> &obj) {
    if (!obj)
      return;
    std::string objName = obj->sceneobject.lock()->_name;
    if (ImGui::BeginPopupContextItem(("ContextMenu##" + objName).c_str())) {
      auto so = obj->sceneobject.lock();
      auto preso = std::dynamic_pointer_cast<Engine::PrefabSceneObject>(so);
      auto nestedScene = std::dynamic_pointer_cast<Engine::Scene>(so);
      if (preso != nullptr) {
        if (ImGui::MenuItem(("Save prefab##" + objName).c_str())) {
          preso->SavePrefab();
        }
      }
      if (preso == nullptr && nestedScene == nullptr && so != nullptr) {
        if (ImGui::MenuItem(("To prefab##" + objName).c_str())) {
          auto newSo = std::make_shared<Engine::PrefabSceneObject>(so);

          if (newSo->instance) {
            newSo->instance->sceneobject = newSo;
          }
          for (auto &child : newSo->Children) {
            if (child) {
              child->Parent = newSo;
            }
          }


          if (auto parent = newSo->Parent.lock()) {
            auto it =
                std::find(parent->Children.begin(), parent->Children.end(), so);
            if (it != parent->Children.end()) {
              *it = newSo;
            }
          } 

          if (sceneObject == so) {
            sceneObject = newSo;
          }
          ImGui::EndPopup();
          return;
        }
      }
      if (nestedScene != nullptr) {
        if (ImGui::MenuItem(("Reload scene file##" + objName).c_str())) {
          pendingReload = nestedScene;
          openReloadPopup = true;
        }       
        if (ImGui::MenuItem(("Save nested scene file##" + objName).c_str()))
          nestedScene->Save();
      }
      ImGui::InputText(("Scene file##" + objName).c_str(), nestedPathBuf, sizeof(nestedPathBuf));
      if (ImGui::MenuItem(("Add nested scene here##" + objName).c_str())) {
        Engine::Engine::instance->current_scene->InstantiateScene(nestedPathBuf, so);
      }
      if (ImGui::MenuItem(("New Object##" + objName).c_str())) {
        auto o = std::make_shared<Engine::Object>();
        Engine::Engine::instance->current_scene->Instantiate(
            o, obj->sceneobject.lock());
      }
      if (ImGui::MenuItem(("New Prefab Object##" + objName).c_str())) {
        auto o = std::make_shared<Engine::Object>();
        Engine::Engine::instance->current_scene->Instantiate(
            o, obj->sceneobject.lock(), true);
      }
      if (ImGui::MenuItem(("Delete##" + objName).c_str())) {
        Engine::Engine::instance->current_scene->RemoveObject(
            obj->sceneobject.lock());
        ImGui::EndPopup();
        return;
      }
      if (ImGui::MenuItem(("Delete Cascade##" + objName).c_str())) {
        Engine::Engine::instance->current_scene->RemoveObjectCascade(
            obj->sceneobject.lock());
        ImGui::EndPopup();
        return;
      }
      ImGui::EndPopup();
    }
  }
};
} // namespace Editor
