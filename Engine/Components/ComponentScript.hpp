#pragma once

#include "Interfaces/IComponent.hpp"
#include "Interfaces/IIdentifiable.hpp"
namespace Engine {
class ComponentScript : public IComponent {
public:
  REGISTER_CLASS(ComponentScript);
  ENGINE_COMPONENT_TYPE GetType() override;
};
} // namespace Engine
