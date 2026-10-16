#include "ComponentControllable.hpp"
#include "InputSystem.hpp"
#include "Object.hpp"
#include "Util/LoggerUtil.hpp"
#include <GLFW/glfw3.h>
void Editor::ComponentControllable::Setup() {
  auto o = object.lock();
  auto cam = o->GetComponent<Engine::Graphics::CameraComponent>();
  keybindMoveForward = Engine::InputSystem::NewKeybind(
      "move-forward", GLFW_KEY_W, GLFW_KEY_W, 0, Engine::KeypressType::HOLD,
      [this, o, cam] {
        o->_position += cam->front * movespeed * GetDeltaTime();
      });
  keybindMoveForward->isEnabled = true;
  keybindMoveBackward = Engine::InputSystem::NewKeybind(
      "move-backward", GLFW_KEY_S, GLFW_KEY_S, 0, Engine::KeypressType::HOLD,
      [this, o, cam] {
        o->_position -= cam->front * movespeed * GetDeltaTime();
      });

  keybindMoveBackward->isEnabled = true;
  keybindMoveLeft = Engine::InputSystem::NewKeybind(
      "move-left", GLFW_KEY_A, GLFW_KEY_A, 0, Engine::KeypressType::HOLD,
      [this, o, cam] {
        o->_position -= cam->right * movespeed * GetDeltaTime();
      });

  keybindMoveLeft->isEnabled = true;
  keybindMoveRight = Engine::InputSystem::NewKeybind(
      "move-right", GLFW_KEY_D, GLFW_KEY_D, 0, Engine::KeypressType::HOLD,
      [this, o, cam] {
        o->_position += cam->right * movespeed * GetDeltaTime();
      });

  keybindMoveRight->isEnabled = true;
  keybindLook = Engine::InputSystem::NewKeybindMouseMove(
      "look", Engine::MOVE_TYPE::NORMAL, 0, 0, 0, [this](int x, int y) {
        if (isNew) {
          oldx = x;
          oldy = y;
          isNew = false;
        }
        auto deltax = (x - oldx) * sensitivity;
        auto deltay = (y - oldy) * sensitivity;
        oldx = x;
        oldy = y;
        auto o = object.lock();
        o->_rotation =
            glm::angleAxis(glm::radians(-deltax), glm::vec3(0, 1, 0)) *
            o->_rotation;
        float newPitch = glm::clamp(currentPitch - deltay, -89.0f, 89.0f);
        float clampedDelta = newPitch - currentPitch;
        currentPitch = newPitch;
        o->_rotation = o->_rotation * glm::angleAxis(glm::radians(clampedDelta),
                                                     glm::vec3(1, 0, 0));
        o->_rotation = glm::normalize(o->_rotation);
      });
  keybindLook->isEnabled = false;
  keybindControll = Engine::InputSystem::NewKeybind(
      "keybind-control", GLFW_KEY_ESCAPE, GLFW_KEY_ESCAPE, 0,
      Engine::KeypressType::PRESS, [this]() {
        isNew = true;
        if (keybindLook->isEnabled) {
          Engine::InputSystem::SetMouseEnabled(true);
          keybindLook->isEnabled = false;
        } else {
          Engine::InputSystem::SetMouseEnabled(false);
          keybindLook->isEnabled = true;
        }
      });
  keybindControll->isEnabled = true;
}
