#pragma once

#include "Interfaces/IComponent.hpp"
#include "Interfaces/IIdentifiable.hpp"
#include <glm/glm.hpp>
#include <imgui.h>
#include <memory>
namespace Engine {
namespace Physics {
using json = nlohmann::json;
class ComponentPhysics : public IComponent,
                         public std::enable_shared_from_this<ComponentPhysics> {
  REGISTER_CLASS(ComponentPhysics);

public:
  json ToJson() override;

  void FromJson(json &js) override;
  void Update() override;
  void Setup() override;
  void End() override;
  void SetVelocities(glm::dvec3 newv) { velocities = newv; }
  void SetVelocityX(double newx) { velocities.x = newx; }
  void SetVelocityY(double newy) { velocities.y = newy; }
  void SetVelocityZ(double newz) { velocities.z = newz; }
  glm::dvec3 GetVelocities() { return velocities; }
  void Impulse(glm::dvec3 newv) { velocities += newv; }
  bool processPhysics = true;
  bool useGlobalGravity = true;
  bool useGlobalDirection = true;

private:
  glm::dvec3 velocities = {0, 0, 0};
  glm::dvec3 world_direction;
  glm::dvec3 gravity_scale;
};
} // namespace Physics
} // namespace Engine
