#pragma once
#include "glm/glm.hpp"
#include <memory>
#define CMP(x, y)                                                              \
  (std::abs((x) - (y)) <=                                                      \
   FLT_EPSILON * fmaxf(1.0f, std::max<float>(std::abs(x), std::abs(y))))

namespace Engine {
namespace Physics {
class OBB;
struct RaycastResult {
  bool hit;
  float t;
  glm::vec3 point;
  glm::vec3 normal;
};
class Ray {
public:
  glm::vec3 origin;
  glm::vec3 direction;
  Ray(glm::vec3 from, glm::vec3 to) {
    origin = from;
    direction = glm::normalize(to - from);
  }
  Ray() {}
  RaycastResult Intersects(std::shared_ptr<OBB> obb);
};
} // namespace Physics
} // namespace Engine
