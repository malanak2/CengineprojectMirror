#pragma once

#include "ScriptSystem.hpp"
#include "Interfaces/IComponent.hpp"
#include "Interfaces/IIdentifiable.hpp"
namespace Editor {
class [[=Engine::Attr::Script{}]] TestInputComponent : public Engine::IComponent {
  REGISTER_CLASS(TestInputComponent);

public:
  json ToJson() override { return {}; }

  void FromJson(json &js) override {}

  void Update() override;
  void Setup() override;
  void KeyFunc();
};
} // namespace Editor
