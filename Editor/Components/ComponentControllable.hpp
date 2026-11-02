#pragma once

#include "InputSystem.hpp"
#include "Interfaces/IComponent.hpp"
#include "Interfaces/IIdentifiable.hpp"
#include "ScriptSystem.hpp"
namespace Editor {
class [[=Engine::Attr::Script{}]] ComponentControllable : public Engine::IComponent {
  REGISTER_CLASS(ComponentControllable);

public:
  void EarlySetup() override;
  void LateEnd() override;
  ~ComponentControllable() override;
  [[=Engine::Attr::Serialize{}]]float movespeed = 1;
  [[=Engine::Attr::Serialize{}]]float sensitivity = 1;
  bool FollowsPausedExecution() override { return false; }

private:
  std::shared_ptr<Engine::Keybind> keybindMoveForward;
  std::shared_ptr<Engine::Keybind> keybindMoveBackward;
  std::shared_ptr<Engine::Keybind> keybindMoveLeft;
  std::shared_ptr<Engine::Keybind> keybindMoveRight;
  std::shared_ptr<Engine::Keybind> keybindControll;
  std::shared_ptr<Engine::KeybindMouse> keybindLook;
  double oldx = 0, oldy = 0;
  float currentPitch = 0.0f;
  bool isNew = true;
  void ReleaseKeybinds(bool restoreMouse);
  float yaw = 0;
};
} // namespace Editor
