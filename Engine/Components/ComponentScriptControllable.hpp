#pragma once

#include "Components/ComponentScript.hpp"
#include "Interfaces/IIdentifiable.hpp"
namespace Engine {
class ComponentScriptControllable : public ComponentScript {
public:
  REGISTER_CLASS(ComponentScriptControllable);
  void Setup() override;

  void Update() override;

  void FixedUpdate() override;

  void Save() override;

  void Load() override;

  json ToJson() override;

  void FromJson(json &js) override;

private:
  int scancode_forward;
};
} // namespace Engine
