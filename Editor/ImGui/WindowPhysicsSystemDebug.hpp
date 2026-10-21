#pragma once

#include "ImGui/Window.hpp"
#include "Physics/PhysicsSystem.hpp"
#include <imgui.h>
namespace Editor {
class WindowPhysicsSystemDebug : public Window {
public:
  std::string GetName() override { return "WindowPhysicsSystemDebug"; }

protected:
  void draw() override {
    ImGui::Begin(GetName().c_str());
    ImGui::Text(
        "Bounding Boxes: %zu\nPhysicObjects:%zu\nDesiredTicksPerSecond:%f\n",
        Engine::Physics::PhysicsSystem::boundingBoxes.size(),
        Engine::Physics::PhysicsSystem::cphysics.size(),
        Engine::Physics::PhysicsSystem::physics_framerate);
    ImGui::End();
  }
};
} // namespace Editor
