#pragma once

#include "Interfaces/IComponent.hpp"
#include "Interfaces/IIdentifiable.hpp"
#include <nlohmann/detail/macro_scope.hpp>

#include "ScriptSystem.hpp"

namespace Engine {

class TestComponentJson {
public:
  int amount;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TestComponentJson, amount);

class [[=Attr::Script{}]] TestComponent : public IComponent {
  REGISTER_CLASS(TestComponent);

public:
  void Update() override;

  [[=Attr::Serialize{}]]float amount = 0;

private:
};
} // namespace Engine
