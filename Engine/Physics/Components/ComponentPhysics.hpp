#pragma once

#include "Interfaces/IComponent.hpp"
#include <glm/glm.hpp>
#include "ScriptSystem.hpp"
#include <memory>
// SZAUER, Gabor. Game Physics Cookbook. In: . S. 152-192. ISBN 978-1787120815.
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
  class [[=Attr::Script{ .displayname = "Test"}]] ComponentPhysics : public IComponent, public std::enable_shared_from_this<ComponentPhysics> {
    REGISTER_CLASS(ComponentPhysics)
public:
  void UpdatePhysics();
  void EarlySetup() override;
  void LateEnd() override;
  void SetVelocities(glm::dvec3 newv) { velocities = newv; }
  void SetVelocityX(float newx) { velocities.x = newx; }
  void SetVelocityY(float newy) { velocities.y = newy; }
  void SetVelocityZ(float newz) { velocities.z = newz; }
  glm::vec3 GetVelocities() { return velocities; }
  void Impulse(glm::dvec3 newv) { velocities += newv; }
  [[=Attr::Serialize{}]] [[=Attr::Editor{}]] bool processPhysics = true;
  [[=Attr::Serialize{}]] [[=Attr::Editor{}]] bool useGlobalGravity = true;
  [[=Attr::Serialize{}]] [[=Attr::Editor{}]] bool useGlobalDirection = true;
  glm::vec3 oldPos;
  [[=Attr::Serialize{}]] [[=Attr::Editor{}]] float mass = 1;
  [[=Attr::Serialize{}]] [[=Attr::Editor{}]] float cor = 0; 
  [[=Attr::Serialize{}]] [[=Attr::Editor{}]] float friction = 1;
  void ApplyForces();

  void ApplyImpulse(const std::shared_ptr<ComponentPhysics> &B,
                    const std::shared_ptr<CollisionManifold> &M);
  float GetDeltaTime() override;
  void BeginTick();
  void EndTick();
  glm::vec3 GetRenderPosition();
    ComponentPhysics();
  ::json ToJson() override {return {};}
  void FromJson(::json& js) override {}

private:
  [[=Attr::Serialize{}]] [[=Attr::Editor{}]]glm::vec3 velocities = {0, 0, 0};
  [[=Attr::Serialize{}]] [[=Attr::Editor{}]]glm::vec3 world_direction;
  [[=Attr::Serialize{}]] [[=Attr::Editor{}]]glm::vec3 gravity_scale;
  glm::vec3 prevPos = {0, 0, 0};
  glm::vec3 currPos = {0, 0, 0};
  bool hasTickState = false;
};
} // namespace Physics
} // namespace Engine
