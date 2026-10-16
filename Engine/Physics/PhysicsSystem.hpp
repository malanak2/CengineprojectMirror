#pragma once

#include "glm/glm.hpp"

namespace Engine {
namespace Physics {
class PhysicsSystem {
public:
  static glm::dvec3 world_direction;
  static double gravity_scale;
  static double gravity_accel;

private:
};
} // namespace Physics
} // namespace Engine
