#pragma once

#include "Interfaces/IComponent.hpp"
#include "Interfaces/IIdentifiable.hpp"
#include <nlohmann/detail/macro_scope.hpp>
namespace Engine {

class TestComponentJson {
public:
  int amount;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TestComponentJson, amount);

class TestComponent : public IComponent {
  REGISTER_CLASS(TestComponent);

public:
  void Update() override;

  json ToJson() override;

  void FromJson(json &js) override;

  int amount = 0;

private:
};
} // namespace Engine
