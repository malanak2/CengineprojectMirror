#pragma once

#include "Interfaces/IComponent.hpp"
namespace Engine {
class ComponentScript : public IComponent {
public:
  ENGINE_COMPONENT_TYPE GetType() override;
  virtual void RenderImGui() override;
  virtual std::string GetName() override;
};
} // namespace Engine
