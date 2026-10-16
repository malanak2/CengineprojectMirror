#pragma once

#include "InputSystem.hpp"
#include "Interfaces/IComponent.hpp"
#include "Interfaces/IIdentifiable.hpp"
namespace Editor {
class ComponentControllable : public Engine::IComponent {
  REGISTER_CLASS(ComponentControllable);

public:
  json ToJson() override {
    return {{"movespeed", movespeed}, {"sensitivity", sensitivity}};
  }

  void FromJson(json &js) override {
    movespeed = js["movespeed"].get<float>();
    sensitivity = js["sensitivity"].get<float>();
  }
  void Setup() override;
  float movespeed = 1;
  float sensitivity = 1;

private:
  std::shared_ptr<Engine::Keybind> keybindMoveForward;
  std::shared_ptr<Engine::Keybind> keybindMoveBackward;
  std::shared_ptr<Engine::Keybind> keybindMoveLeft;
  std::shared_ptr<Engine::Keybind> keybindMoveRight;
  std::shared_ptr<Engine::Keybind> keybindControll;
  std::shared_ptr<Engine::KeybindMouse> keybindLook;
  int oldx = 0, oldy = 0;
  float currentPitch = 0.0f;
  bool isNew = true;
};
} // namespace Editor
