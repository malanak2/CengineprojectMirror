#pragma once

#include "Physics/Components/ComponentBoundingBox.hpp"
#include "Physics/Components/ComponentPhysics.hpp"
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
  static std::vector<std::pair<std::pair<std::shared_ptr<ComponentPhysics>,
                                         std::shared_ptr<ComponentPhysics>>,
                               std::shared_ptr<CollisionManifold>>>
      collisions;
  static void ApplyStaticImpulse(std::shared_ptr<ComponentPhysics> body,
                                 std::shared_ptr<CollisionManifold> M,
                                 int contactIndex);

  //!
  //! @LinearProjectionPercent Keep between 0.2 and 0.8
  //!
  static float LinearProjectionPercent;
  //!
  //! @PenetrationSlack 0.01 - 0.1
  //!
  static float PenetrationSlack;
  //!
  //! @ImpulseIteration 1 - 20
  //!
  static int ImpulseIteration;
  static void ProcessPhysics();
  static float accumulator;
  static float physics_framerate;

private:
  static void phystick();
};
} // namespace Physics
} // namespace Engine
