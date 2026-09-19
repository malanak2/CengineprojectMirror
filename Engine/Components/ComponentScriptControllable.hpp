#pragma once

#include "Components/ComponentScript.hpp"
#include "Interfaces/IIdentifiable.hpp"
namespace Engine {
class ComponentScriptControllable : public ComponentScript {
  REGISTER_CLASS(ComponentScriptControllable);

public:
  void Setup() override;

  void Update() override;

  void FixedUpdate() override;

  json ToJson() override;

  void FromJson(json &js) override;

private:
  int scancode_forward;
};
} // namespace Engine
