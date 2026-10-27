#include "Plane.hpp"
#include "Components/ComponentBoundingBox.hpp"

// SZAUER, Gabor. Game Physics Cookbook. In: . S. 152-192. ISBN 978-1787120815.
float Engine::Physics::Plane::Classify(std::shared_ptr<OBB> obb) {
  if (!obb)
    return 0.0f;

  glm::vec3 localNormal = glm::transpose(obb->orientation) * normal;

  float r = glm::abs(obb->size.x * localNormal.x) +
            glm::abs(obb->size.y * localNormal.y) +
            glm::abs(obb->size.z * localNormal.z);

  float d = PlaneEquation(obb->position);
  if (glm::abs(d) <= r) {
    return 0.0f;
  } else if (d < -r) {
    return d + r;
  }
  return d - r;
}
