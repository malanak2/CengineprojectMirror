#pragma once

#include "Physics/Components/ComponentBoundingBox.hpp"
#include "glm/glm.hpp" // IWYU pragma: keep
#include <memory>

namespace Engine {
namespace Physics {
class ComponentPhysics;
class PhysicsSystem {
public:
  static glm::vec3 world_direction;
  static glm::vec3 gravity_scale;
  static float gravity_accel;

  static void RegisterCPhysics(std::shared_ptr<ComponentPhysics> c);
  static void UnregisterCPhysics(std::shared_ptr<ComponentPhysics> c);
  static std::vector<std::shared_ptr<ComponentPhysics>> cphysics;
  static void RegisterBBox(std::shared_ptr<ComponentBoundingBox> c);
  static void UnregisterBBox(std::shared_ptr<ComponentBoundingBox> c);
  static std::vector<std::shared_ptr<ComponentBoundingBox>> boundingBoxes;

  static void ProcessPhysics();

private:
};
} // namespace Physics
} // namespace Engine
