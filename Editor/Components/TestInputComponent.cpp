#include "TestInputComponent.hpp"
#include "InputSystem.hpp"
#include "Object.hpp" // IWYU pragma: keep
#include <glm/gtc/quaternion.hpp>

using namespace Editor;

void TestInputComponent::Update() {}
void Editor::TestInputComponent::Setup() {
  auto w =
      Engine::InputSystem::NewKeybind("test-keybind-w", GLFW_KEY_W, GLFW_KEY_W,
                                      0, Engine::KeypressType::HOLD, [this]() {
                                        auto o = object.lock();
                                        o->_position.x += 144 * GetDeltaTime();
                                      });
  w->isEnabled = true;
  auto w_shift = Engine::InputSystem::NewKeybind(
      "test-keybind-w-shift", GLFW_KEY_W, GLFW_KEY_W, GLFW_MOD_SHIFT,
      Engine::KeypressType::HOLD, [this]() {
        auto o = object.lock();
        o->_position.x += 288 * GetDeltaTime();
      });
  w_shift->isEnabled = true;
  auto s =
      Engine::InputSystem::NewKeybind("test-keybind-s", GLFW_KEY_S, GLFW_KEY_S,
                                      0, Engine::KeypressType::HOLD, [this]() {
                                        auto o = object.lock();
                                        o->_position.x -= 144 * GetDeltaTime();
                                      });
  s->isEnabled = true;
  auto a =
      Engine::InputSystem::NewKeybind("test-keybind-a", GLFW_KEY_A, GLFW_KEY_A,
                                      0, Engine::KeypressType::HOLD, [this]() {
                                        auto o = object.lock();
                                        o->_position.z -= 144 * GetDeltaTime();
                                      });
  a->isEnabled = true;
  auto d =
      Engine::InputSystem::NewKeybind("test-keybind-d", GLFW_KEY_D, GLFW_KEY_D,
                                      0, Engine::KeypressType::HOLD, [this]() {
                                        auto o = object.lock();
                                        o->_position.z += 144 * GetDeltaTime();
                                      });
  d->isEnabled = true;

  auto click = Engine::InputSystem::NewKeybindMouseClick(
      "test-mouse-click", GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_LEFT, 0,
      Engine::CLICK_TYPE::CLICK_PRESS, [this]() {
        auto o = object.lock();
        if (o)
          o->_position.y += 1.0f;
      });
  click->isEnabled = true;

  auto move = Engine::InputSystem::NewKeybindMouseMove(
      "test-mouse-drag", Engine::MOVE_TYPE::BUTTON_HELD,
      GLFW_MOUSE_BUTTON_RIGHT, GLFW_MOUSE_BUTTON_RIGHT, 0,
      [this](double x, double y) {
        auto o = object.lock();
        if (o) {
          float yaw = (float)Engine::InputSystem::GetMouseDeltaX() * 0.2f;
          float pitch = (float)Engine::InputSystem::GetMouseDeltaY() * 0.2f;
          o->_rotation =
              glm::angleAxis(glm::radians(-yaw), glm::vec3(0, 1, 0)) *
              o->_rotation;
          o->_rotation = o->_rotation * glm::angleAxis(glm::radians(pitch),
                                                       glm::vec3(1, 0, 0));
          o->_rotation = glm::normalize(o->_rotation);
        }
      });
  move->isEnabled = true;

  auto scroll = Engine::InputSystem::NewKeybindMouseScroll(
      "test-mouse-scroll", Engine::SCROLL_TYPE::SCROLL_ANY, 0,
      [this](double xoffset, double yoffset) {
        auto o = object.lock();
        if (o) {
          o->_position.y += (float)yoffset * 10.0f;
        }
      });
  scroll->isEnabled = true;
}
