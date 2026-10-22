#pragma once

#include "Engine.hpp"
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

    const float current =
        Engine::Engine::isSimulationEnabled
            ? Engine::Physics::PhysicsSystem::currentTicksPerSecond
            : 0.0f;
    ImGui::Text("Bounding Boxes: %zu",
                Engine::Physics::PhysicsSystem::boundingBoxes.size());
    ImGui::Text("PhysicObjects: %zu",
                Engine::Physics::PhysicsSystem::cphysics.size());
    ImGui::InputFloat("DesiredTicksPerSecond:",
                      &Engine::Physics::PhysicsSystem::physics_framerate);
    ImGui::Text("CurrentTicksPerSecond: %.1f\n", current);
    ImGui::Checkbox("Interpolate rendering",
                    &Engine::Physics::PhysicsSystem::interpolateRendering);
    ImGui::End();
  }
};
} // namespace Editor
