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
  static void ApplyStaticImpulse(const std::shared_ptr<ComponentPhysics> &body,
                                 const std::shared_ptr<CollisionManifold> &M);

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
  static float currentTicksPerSecond;
  static bool interpolateRendering;
  static float GetInterpolationAlpha();
  struct Contact {
    std::weak_ptr<ComponentBoundingBox> a, b;
    bool trigger = false;
    int missed = 0; // consecutive ticks without contact (leave grace)
  };

  using ContactKey =
      std::pair<const ComponentBoundingBox *, const ComponentBoundingBox *>;
  enum class ContactEvent { Enter, Stay, Leave };

private:
  static void phystick();
  static int tickCount;
  static float tickWindow;
  static std::map<ContactKey, Contact> previousContacts;
  static std::map<ContactKey, Contact> currentContacts;
  static bool ReportContact(const std::shared_ptr<ComponentBoundingBox> &a,
                            const std::shared_ptr<ComponentBoundingBox> &b,
                            bool trigger);
  static void DispatchContactEvents();
  static void
  FireContactEvent(const std::shared_ptr<ComponentBoundingBox> &self,
                   const std::shared_ptr<ComponentBoundingBox> &other,
                   ContactEvent type, bool trigger);
};
} // namespace Physics
} // namespace Engine
