#include "ComponentControllable.hpp"
#include "InputSystem.hpp"
#include "Object.hpp"
#include "Util/LoggerUtil.hpp"
#include <GLFW/glfw3.h>
void Editor::ComponentControllable::EarlySetup() {
  auto o = object.lock();
  auto cam = o->GetComponent<Engine::Graphics::CameraComponent>();

  auto step = [this](float forward, float strafe) {
    auto o = object.lock();
    if (!o)
      return;
    glm::vec3 front = o->GetRotation() * glm::vec3(0, 0, -1);
    glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));
    o->AddWorldPosition((front * forward + right * strafe) * movespeed *
                        GetDeltaTime());
  };
  keybindMoveForward = Engine::InputSystem::NewKeybind(
      "move-forward", GLFW_KEY_W, GLFW_KEY_W, 0, Engine::KeypressType::HOLD,
      [step] { step(1, 0); }, false);
  keybindMoveForward->isEnabled = true;
  keybindMoveBackward = Engine::InputSystem::NewKeybind(
      "move-backward", GLFW_KEY_S, GLFW_KEY_S, 0, Engine::KeypressType::HOLD,
      [step] { step(-1, 0); }, false);

  keybindMoveBackward->isEnabled = true;
  keybindMoveLeft = Engine::InputSystem::NewKeybind(
      "move-left", GLFW_KEY_A, GLFW_KEY_A, 0, Engine::KeypressType::HOLD,
      [step] { step(0, -1); }, false);

  keybindMoveLeft->isEnabled = true;
  keybindMoveRight = Engine::InputSystem::NewKeybind(
      "move-right", GLFW_KEY_D, GLFW_KEY_D, 0, Engine::KeypressType::HOLD,
      [step] { step(0, 1); }, false);

  keybindMoveRight->isEnabled = true;
  keybindLook = Engine::InputSystem::NewKeybindMouseMove(
      "look", Engine::MOVE_TYPE::NORMAL, 0, 0, 0,
      [this](double x, double y) {
        auto o = object.lock();
        if (!o)
          return;
        if (isNew) {
          oldx = x;
          oldy = y;
          isNew = false;
          // pick up whatever rotation the object has now (inspector edits,
          // etc.)
          glm::vec3 f = o->GetLocalRotation() * glm::vec3(0, 0, -1);
          this->yaw = glm::degrees(std::atan2(-f.x, -f.z));
          currentPitch = glm::degrees(std::asin(glm::clamp(f.y, -1.0f, 1.0f)));
        }
        yaw -= float(x - oldx) * sensitivity;
        currentPitch = glm::clamp(currentPitch - float(y - oldy) * sensitivity,
                                  -89.0f, 89.0f);
        oldx = x;
        oldy = y;
        o->SetRotation(glm::normalize(
            glm::angleAxis(glm::radians(yaw), glm::vec3(0, 1, 0)) *
            glm::angleAxis(glm::radians(currentPitch), glm::vec3(1, 0, 0))));
      },
      false);
  keybindLook->isEnabled = false;
  keybindControll = Engine::InputSystem::NewKeybind(
      "keybind-control", GLFW_KEY_ESCAPE, GLFW_KEY_ESCAPE, 0,
      Engine::KeypressType::PRESS,
      [this]() {
        isNew = true;
        if (keybindLook->isEnabled) {
          Engine::InputSystem::SetMouseEnabled(true);
          keybindLook->isEnabled = false;
        } else {
          Engine::InputSystem::SetMouseEnabled(false);
          keybindLook->isEnabled = true;
        }
      },
      false);
  keybindControll->isEnabled = true;
}
void Editor::ComponentControllable::LateEnd() { ReleaseKeybinds(true); }
Editor::ComponentControllable::~ComponentControllable() {
  ReleaseKeybinds(false);
}
void Editor::ComponentControllable::ReleaseKeybinds(bool restoreMouse) {
  if (keybindLook && keybindLook->isEnabled)
    Engine::InputSystem::SetMouseEnabled(true);

  for (auto *keybind :
       {&keybindMoveForward, &keybindMoveBackward, &keybindMoveLeft,
        &keybindMoveRight, &keybindControll}) {
    if (*keybind) {
      (*keybind)->isEnabled = false;
      (*keybind)->f = nullptr;
      keybind->reset();
    }
  }
  if (keybindLook) {
    keybindLook->isEnabled = false;
    keybindLook.reset();
  }
}
