#pragma once

#include "Interfaces/IComponent.hpp"
#include "Interfaces/IIdentifiable.hpp"
#include "Physics/PhysicsSystem.hpp"
#include <glm/glm.hpp>
#include <imgui.h>
#include <memory>
namespace Engine {
namespace Physics {
class CollisionManifold {
public:
  bool colliding;
  glm::vec3 normal;
  float depth;
  std::vector<glm::vec3> contacts;
  void ResetCollisionManifold();
};
using json = nlohmann::json;
class ComponentPhysics : public IComponent,
                         public std::enable_shared_from_this<ComponentPhysics> {
  REGISTER_CLASS(ComponentPhysics);

public:
  json ToJson() override;

  void FromJson(json &js) override;
  void UpdatePhysics();
  void Setup() override;
  void End() override;
  void SetVelocities(glm::dvec3 newv) { velocities = newv; }
  void SetVelocityX(float newx) { velocities.x = newx; }
  void SetVelocityY(float newy) { velocities.y = newy; }
  void SetVelocityZ(float newz) { velocities.z = newz; }
  glm::vec3 GetVelocities() { return velocities; }
  void Impulse(glm::dvec3 newv) { velocities += newv; }
  bool processPhysics = true;
  bool useGlobalGravity = true;
  bool useGlobalDirection = true;
  glm::vec3 oldPos;
  float bounce = 1;
  float mass = 1;
  float cor; // Coefficient of restitution
  float friction = 1;
  void ApplyForces();

  void ApplyImpulse(std::shared_ptr<ComponentPhysics> B,
                    std::shared_ptr<CollisionManifold> M, int c);
  float GetDeltaTime() override;

private:
  glm::vec3 velocities = {0, 0, 0};
  glm::vec3 world_direction;
  glm::vec3 gravity_scale;
};
} // namespace Physics
} // namespace Engine
