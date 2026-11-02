#pragma once

#include "Interfaces/IComponent.hpp"
#include "Interfaces/IIdentifiable.hpp"
#include <implot.h>

#include "ScriptSystem.hpp"

namespace Editor {
class [[=Engine::Attr::Script{}]] ComponentTestTrigger : public Engine::IComponent {
  REGISTER_CLASS(ComponentTestTrigger);

public:
  virtual ~ComponentTestTrigger() override = default;
  void Setup() override;
  void End() override;
};
} // namespace Editor
