#pragma once

#include "Interfaces/IComponent.hpp"
namespace Engine {
class ComponentScript : public IComponent {
public:
  ENGINE_COMPONENT_TYPE GetType() override;
  virtual void RenderImGui();
  virtual std::string GetName();
};
} // namespace Engine
