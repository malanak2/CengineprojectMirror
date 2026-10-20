#pragma once

#include "Physics/Ray.hpp"
#include <glm/glm.hpp>
namespace Engine {
namespace Physics {
class Line {
public:
  glm::vec3 start;
  glm::vec3 end;
  float Magnitude(const glm::vec2 &v) { return sqrtf(glm::dot(v, v)); }
  float Magnitude(const glm::vec3 &v) { return sqrtf(glm::dot(v, v)); }
  float MagnitudeSq(const glm::vec2 &v) { return glm::dot(v, v); }
  float MagnitudeSq(const glm::vec3 &v) { return glm::dot(v, v); }
  float Length() { return Magnitude(start - end); }
  float LengthSq() { return MagnitudeSq(start - end); }
  bool Linetest(std::shared_ptr<OBB> obb) {
    Ray ray;
    ray.origin = start;
    ray.direction = glm::normalize(end - start);
    auto res = ray.Intersects(obb);
    auto t = res.t;

    return t >= 0 && t * t <= LengthSq();
  }
};
} // namespace Physics
} // namespace Engine
