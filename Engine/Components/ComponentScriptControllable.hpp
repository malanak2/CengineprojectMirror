#pragma once

#include "Components/ComponentScript.hpp"
namespace Engine {
class ComponentScriptControllable : public ComponentScript {
public:
  void Setup() override;

  void Update() override;

  void FixedUpdate() override;

  void Save() override;

  void Load() override;

  ENGINE_COMPONENT_TYPE GetType() override;

  json ToJson() override;

  void FromJson(json &js) override;

  void RenderImGui() override;
  std::string GetName() override;

private:
  int scancode_forward;
};
} // namespace Engine
