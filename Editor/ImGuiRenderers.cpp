#include "ImGuiRenderers.hpp"
#include "Components/InvalidComponent.hpp"
#include "Components/TestComponent.hpp"
#include "Graphics/Components/CameraComponent.hpp"
#include "Graphics/Components/ComponentAnimator.hpp"
#include "Graphics/Components/ComponentRenderable.hpp"
#include "Graphics/Graphics.hpp" // IWYU pragma: keep
#include "Graphics/Uniforms/UniformFloatVector.hpp"
#include "ImGuiMacros.hpp"
#include "ScriptSystem.hpp"
#include "Util/FileUtil.hpp"
#include "Util/LoggerUtil.hpp"
#include <imgui.h>
#include <memory>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>

namespace Editor::ImGuiR {

std::shared_ptr<ImGuiUniformRenderer> ImGuiUniformRenderer::instance =
    std::make_shared<ImGuiUniformRenderer>();
std::shared_ptr<ImGuiComponentRenderer> ImGuiComponentRenderer::instance =
    std::make_shared<ImGuiComponentRenderer>();

void ImGuiUniformRenderer::Register(
    std::string_view type,
    void (*func)(std::shared_ptr<Engine::Graphics::IUniform>)) {
  if (funcs.contains(type)) {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Tried to re-register type");
    return;
  }
  funcs.insert({type, func});
}
void ImGuiUniformRenderer::Render(
    std::shared_ptr<Engine::Graphics::IUniform> uni) {
  ZoneScoped;
  auto t = uni->GetName();
  if (funcs.contains(t)) {
    funcs[t](uni);
  } else {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Typeid {} not registered", t);
  }
}

void ImGuiUniformRenderer::Init() {
  ZoneScoped;
  IMGUI_REGISTER_UNIFORM(Engine::Graphics::UniformFloatVector, {
    auto uniformCast =
        std::static_pointer_cast<Engine::Graphics::UniformFloatVector>(uniform);
    std::string key = *uniformCast->info.name;
    switch (uniformCast->data.size()) {
    case 1: {
      ImGui::InputFloat(&key[0], &uniformCast->data[0]);
      break;
    }
    case 2: {
      ImGui::InputFloat2(&key[0], &uniformCast->data[0]);
      break;
    }
    case 3: {
      ImGui::InputFloat3(&key[0], &uniformCast->data[0]);
      break;
    }
    case 4: {
      if (key == "color") {
        ImGui::ColorPicker4(&key[0], &uniformCast->data[0]);
      } else {
        ImGui::InputFloat4(&key[0], &uniformCast->data[0]);
      }
      break;
    }
    default: {
      ImGui::Text("Unsupported float uniform %s of length %zu", &key[0],
                  uniformCast->data.size());
      break;
    }
    }
  });
}

void ImGuiComponentRenderer::Register(
    std::string_view type, void (*func)(std::shared_ptr<Engine::IComponent>)) {
  if (funcs.contains(type)) {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Tried to re-register Component");
    return;
  }
  funcs.insert({type, func});
}
void ImGuiComponentRenderer::Render(std::shared_ptr<Engine::IComponent> uni) {
  ZoneScoped;
  auto t = uni->GetName();
  if (funcs.contains(t)) {
    funcs[t](uni);
  } else {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Typeid {} not registered", t);
  }
}

void ImGuiComponentRenderer::Init() {
  ZoneScoped;
  IMGUI_REGISTER_COMPONENT(Engine::Graphics::ComponentRenderable, {
    ImGui::InputText("Model path", &component->_model_path[0], 100);
    ImGui::InputText("Material path", &component->_material_path[0], 100);

    if (ImGui::CollapsingHeader("Uniforms")) {
      for (auto &[key, val] : component->_uniforms) {
        ImGuiUniformRenderer::instance->Render(val);
      }
    }
  });
  IMGUI_REGISTER_COMPONENT(Engine::InvalidComponent,
                           { ImGui::Text("Invalid Component"); })

  IMGUI_REGISTER_COMPONENT(Engine::Graphics::CameraComponent, {
    ImGui::InputFloat("Near", &component->near);
    ImGui::InputFloat("Far", &component->far);
  });

  IMGUI_REGISTER_COMPONENT(Engine::TestComponent, {
    ImGui::InputFloat("Amount", &component->amount);
  });

  IMGUI_REGISTER_COMPONENT(Engine::Graphics::ComponentAnimator, {
    if (ImGui::BeginCombo("Animation",
                          component->GetCurrentAnimation().c_str())) {
      for (const auto &[name, anim] : component->animators) {
        bool isSelected = (component->GetCurrentAnimation() == name);
        if (ImGui::Selectable(name.c_str(), isSelected)) {
          component->SetAnimation(name);
        }
      }
      bool isSelected = (component->GetCurrentAnimation() == "");
      if (ImGui::Selectable("None", isSelected)) {
        component->SetAnimation("");
      }
      ImGui::EndCombo();
    }
    if (component->GetIsPlaying()) {
      if (ImGui::Button("Pause")) {
        component->SetIsPlaying(false);
      }
    } else {
      if (ImGui::Button("Play")) {
        component->SetIsPlaying(true);
      }
    }
  });
}

std::shared_ptr<Engine::SceneObject> ImGuiRenderer::sceneObject = nullptr;
std::shared_ptr<Engine::SceneObject> ImGuiRenderer::newObjectParent = nullptr;
bool ImGuiRenderer::wasSavePressedThisFrame = false;
char ImGuiRenderer::namebuf[64] = "";
char ImGuiRenderer::matbuf[64] = "";
float ImGuiRenderer::coords[3] = {0, 0, 0};
float ImGuiRenderer::rotation[4] = {0, 0, 0, 0};

void ImGuiRenderer::ShowSceneObjectMenu(
    std::vector<std::shared_ptr<Engine::SceneObject>> *sceneObjects) {
  ZoneScoped;
  if (!sceneObjects)
    return;
  for (auto &obj : *sceneObjects) {
    if (!obj || !obj->instance)
      continue;
    ImGui::PushID(obj.get());

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowOverlap;
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

void ImGuiRenderer::RenderSceneView(std::shared_ptr<Engine::Scene> scene) {
  ZoneScoped;
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
  IMGUI_CHECKBOX("Vsync", Config::inst->graphics->enableVsync, [](bool state) {
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
// https://en.cppreference.com/cpp/string/byte/tolower
std::string str_tolower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return s;
}
void ImGuiRenderer::RenderObjectInspector() {
  ZoneScoped;
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
        ImGuiComponentRenderer::instance->Render(comp);
      }
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
      sceneObject->instance->AddComponent(name);
    }
  }
  ImGui::End();
}
} // namespace Editor::ImGuiR
