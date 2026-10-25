#include "ImGuiRenderers.hpp"
#include "Components/ComponentControllable.hpp"
#include "Components/InvalidComponent.hpp"
#include "Components/TestComponent.hpp"
#include "Components/TestInputComponent.hpp"
#include "Graphics/Components/CameraComponent.hpp"
#include "Graphics/Components/ComponentAnimator.hpp"
#include "Graphics/Components/ComponentRenderable.hpp"
#include "Graphics/Components/ComponentSocket.hpp"
#include "Graphics/Graphics.hpp" // IWYU pragma: keep
#include "Graphics/Uniforms/UniformFloatVector.hpp"
#include "Physics/Components/ComponentBoundingBox.hpp"
#include "Physics/Components/ComponentPhysics.hpp"
#include "Util/LoggerUtil.hpp"
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <memory>
#include <set>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>

namespace Editor::ImGuiR {

std::shared_ptr<ImGuiUniformRenderer> ImGuiUniformRenderer::instance =
    std::make_shared<ImGuiUniformRenderer>();
std::shared_ptr<ImGuiComponentRenderer> ImGuiComponentRenderer::instance =
    std::make_shared<ImGuiComponentRenderer>();

void ImGuiUniformRenderer::Register(
    std::string_view type,
    void (*func)(const std::shared_ptr<Engine::Graphics::IUniform> &)) {
  if (funcs.contains(type)) {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Tried to re-register type");
    return;
  }
  funcs.insert({type, func});
}
void ImGuiUniformRenderer::Render(
    const std::shared_ptr<Engine::Graphics::IUniform> &uni) {
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
    std::string_view type,
    void (*func)(const std::shared_ptr<Engine::IComponent> &)) {
  if (funcs.contains(type)) {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Tried to re-register Component");
    return;
  }
  funcs.insert({type, func});
}
void ImGuiComponentRenderer::Render(
    const std::shared_ptr<Engine::IComponent> &uni) {
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

    if (component->model && ImGui::CollapsingHeader("Sub-Meshes")) {
      std::set<std::string> seenNames;
      const auto &mList = component->model->GetMeshes();
      for (size_t i = 0; i < mList.size(); ++i) {
        const std::string &meshName = mList[i].name;
        if (!meshName.empty() && seenNames.contains(meshName)) {
          continue;
        }
        if (!meshName.empty()) {
          seenNames.insert(meshName);
        }
        std::string label =
            meshName.empty() ? ("Mesh " + std::to_string(i)) : meshName;
        label += "##" + std::to_string(i);
        bool enabled = component->IsMeshEnabled(meshName);
        if (ImGui::Checkbox(label.c_str(), &enabled)) {
          component->SetMeshEnabled(meshName, enabled);
        }
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
      for (const auto &[name, anim] : component->animations) {
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

    if (ImGui::CollapsingHeader("Socket Attachments")) {
      auto sockets = component->GetSockets();
      bool changed = false;
      int toDelete = -1;

      for (size_t i = 0; i < sockets.size(); ++i) {
        ImGui::PushID(static_cast<int>(i));
        std::string headerName =
            "Socket " + std::to_string(i) + ": " +
            (sockets[i].targetNode.empty() ? "(Empty)"
                                           : sockets[i].targetNode) +
            " -> " +
            (sockets[i].targetBone.empty() ? "(None)" : sockets[i].targetBone);
        if (ImGui::TreeNode(headerName.c_str())) {
          if (ImGui::Checkbox("Enabled", &sockets[i].enabled)) {
            changed = true;
          }
          char nodeBuf[128];
          strncpy(nodeBuf, sockets[i].targetNode.c_str(), sizeof(nodeBuf));
          nodeBuf[sizeof(nodeBuf) - 1] = 0;
          if (ImGui::InputText("Target Node", nodeBuf, sizeof(nodeBuf))) {
            sockets[i].targetNode = nodeBuf;
            changed = true;
          }

          char boneBuf[128];
          strncpy(boneBuf, sockets[i].targetBone.c_str(), sizeof(boneBuf));
          boneBuf[sizeof(boneBuf) - 1] = 0;
          if (ImGui::InputText("Target Bone", boneBuf, sizeof(boneBuf))) {
            sockets[i].targetBone = boneBuf;
            changed = true;
          }

          if (ImGui::DragFloat3("Position Offset",
                                &sockets[i].offsetPosition[0], 0.1f)) {
            changed = true;
          }
          if (ImGui::DragFloat3("Rotation Offset (Deg)",
                                &sockets[i].offsetRotation[0], 1.0f)) {
            changed = true;
          }
          if (ImGui::DragFloat3("Scale Offset", &sockets[i].offsetScale[0],
                                0.01f)) {
            changed = true;
          }

          if (ImGui::Button("Delete Socket")) {
            toDelete = static_cast<int>(i);
          }
          ImGui::TreePop();
        }
        ImGui::PopID();
      }

      if (toDelete >= 0 && toDelete < (int)sockets.size()) {
        sockets.erase(sockets.begin() + toDelete);
        changed = true;
      }

      if (ImGui::Button("Add Socket Attachment")) {
        Engine::Graphics::SocketAttachment newSocket;
        sockets.push_back(newSocket);
        changed = true;
      }

      if (changed) {
        component->SetSockets(sockets);
      }
    }
  });

  IMGUI_REGISTER_COMPONENT(Engine::Graphics::ComponentSocket, {
    char objBuf[128];
    strncpy(objBuf, component->target_object.c_str(), sizeof(objBuf));
    objBuf[sizeof(objBuf) - 1] = 0;
    if (ImGui::InputText("Target Object", objBuf, sizeof(objBuf))) {
      component->target_object = objBuf;
    }

    char boneBuf[128];
    strncpy(boneBuf, component->target_bone.c_str(), sizeof(boneBuf));
    boneBuf[sizeof(boneBuf) - 1] = 0;
    if (ImGui::InputText("Target Bone", boneBuf, sizeof(boneBuf))) {
      component->target_bone = boneBuf;
    }

    ImGui::DragFloat3("Position Offset", &component->offset_position[0], 0.1f);
    ImGui::DragFloat3("Rotation Offset (Deg)", &component->offset_rotation[0],
                      1.0f);
    ImGui::DragFloat3("Scale Offset", &component->offset_scale[0], 0.01f);
  });
  IMGUI_REGISTER_COMPONENT(Editor::TestInputComponent, {

                                                       });
  IMGUI_REGISTER_COMPONENT(Engine::Physics::ComponentPhysics, {
    auto o = component->object.lock();
    auto vel = component->GetVelocities();
    ImGui::Text("VelX %f", vel.x);
    ImGui::Text("VelY %f", vel.y);
    ImGui::Text("VelZ %f", vel.z);
    ImGui::InputFloat("Friction", &component->friction);
    ImGui::InputFloat("Mass", &component->mass);
    ImGui::InputFloat("Cor", &component->cor);
    static double joltx = 0;
    static double jolty = 0;
    static double joltz = 0;
    ImGui::InputDouble("Jolt input X", &joltx);
    ImGui::InputDouble("Jolt input Y", &jolty);
    ImGui::InputDouble("Jolt input Z", &joltz);
    if (ImGui::Button("Jolt")) {
      component->Impulse(glm::dvec3{joltx, jolty, joltz});
    }
    if (ImGui::Button("Set")) {
      component->SetVelocities(glm::dvec3{joltx, jolty, joltz});
    }
  });
  IMGUI_REGISTER_COMPONENT(ComponentControllable, {
    ImGui::InputFloat("Movespeed", &component->movespeed);
    ImGui::InputFloat("Sensitivity", &component->sensitivity);
  });
  IMGUI_REGISTER_COMPONENT(Engine::Physics::ComponentBoundingBox, {
    ImGui::InputFloat3("Position offset", &component->position_offset[0]);
    static glm::vec3 currentAngles = glm::vec3(0.0f);
    static glm::quat lastRotation = glm::identity<glm::quat>();

    if (component->orientation_offset != lastRotation) {
      currentAngles =
          glm::degrees(glm::eulerAngles(component->orientation_offset));
      lastRotation = component->orientation_offset;
    }

    glm::vec3 newAngles = currentAngles;

    if (ImGui::DragFloat3("Rotation##BoundingBox", &newAngles[0], 1.0f)) {
      currentAngles = newAngles;

      glm::vec3 radians = glm::radians(newAngles);
      component->orientation_offset = glm::quat(radians);
      component->orientation_offset =
          glm::normalize(component->orientation_offset);

      lastRotation = component->orientation_offset;
    }
    ImGui::InputFloat3("Size", &component->size[0]);
    ImGui::Checkbox("Trigger", &component->isTrigger);
  });
}
} // namespace Editor::ImGuiR
