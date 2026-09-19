#pragma once

#include "Interfaces/IComponent.hpp"
#include "Interfaces/IIdentifiable.hpp"
namespace Engine {
class InvalidComponent : public IComponent {
  REGISTER_CLASS(InvalidComponent)
public:
  json ToJson() override;

  void FromJson(json &js) override;
};

}; // namespace Engine
