#pragma once

#include "Components/TestComponent.hpp"
#include "ScriptSystem.hpp"
namespace Engine {
class ScriptRegistrar {
public:
  static void RegisterCommonComponents();
  static void RegisterGraphicComponents();
  static void RegisterComponents();
};
} // namespace Engine
