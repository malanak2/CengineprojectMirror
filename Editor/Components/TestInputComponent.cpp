#include "TestInputComponent.hpp"
#include "InputSystem.hpp"
#include "Object.hpp" // IWYU pragma: keep

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
}
