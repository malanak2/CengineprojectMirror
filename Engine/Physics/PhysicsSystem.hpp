#pragma once

#include "glm/glm.hpp" // IWYU pragma: keep
#include <memory>

namespace Engine {
namespace Physics {
class ComponentCollider;
class ComponentPhysics;
class PhysicsSystem {
public:
  static glm::dvec3 world_direction;
  static glm::dvec3 gravity_scale;
  static double gravity_accel;
  static void RegisterCollider(std::shared_ptr<ComponentCollider> c);
  static void UnregisterCollider(std::shared_ptr<ComponentCollider> c);
  static std::vector<std::shared_ptr<ComponentCollider>> colliders;

  static void RegisterCPhysics(std::shared_ptr<ComponentPhysics> c);
  static void UnregisterCPhysics(std::shared_ptr<ComponentPhysics> c);
  static std::vector<std::shared_ptr<ComponentPhysics>> cphysics;

  static void ProcessPhysics();

private:
};
} // namespace Physics
} // namespace Engine
