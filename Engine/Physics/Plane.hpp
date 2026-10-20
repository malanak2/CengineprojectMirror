#pragma once
#include "glm/glm.hpp"
namespace Engine {
namespace Physics {
class Plane {
public:
  glm::vec3 normal;
  float distance;
  inline Plane() : normal(1, 0, 0) {}
  inline Plane(const glm::vec3 &n, float d) : normal(n), distance(d) {}
  float PlaneEquation(const glm::vec3 &pt) {
    return glm::dot(pt, normal) - distance;
  }
};
} // namespace Physics
} // namespace Engine
