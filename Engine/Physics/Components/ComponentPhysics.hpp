#pragma once

#include "Interfaces/IComponent.hpp"
#include "Interfaces/IIdentifiable.hpp"
#include "Physics/PhysicsSystem.hpp"
#include <glm/glm.hpp>
namespace Engine {
namespace Physics {
class ComponentPhysics : public IComponent {
  REGISTER_CLASS(ComponentPhysics);

public:
  json ToJson() override;

  void FromJson(json &js) override;
  void Update() override;
  void SetVelocities(glm::dvec3 newv) { velocities = newv; }
  glm::dvec3 GetVelocities() { return velocities; }
  void Impulse(glm::dvec3 newv) { velocities += newv; }

private:
  glm::dvec3 velocities = {0, 0, 0};
  glm::dvec3 world_direction = PhysicsSystem::world_direction;
  double gravity_scale = PhysicsSystem::gravity_scale;
};
} // namespace Physics
} // namespace Engine
