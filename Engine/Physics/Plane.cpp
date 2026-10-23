#include "Plane.hpp"
#include "Components/ComponentBoundingBox.hpp"

float Engine::Physics::Plane::Classify(std::shared_ptr<OBB> obb) {
  if (!obb)
    return 0.0f;

  glm::vec3 localNormal = glm::transpose(obb->orientation) * normal;

  // 2. Compute correct OBB projected radius using local space normal
  float r = glm::abs(obb->size.x * localNormal.x) +
            glm::abs(obb->size.y * localNormal.y) +
            glm::abs(obb->size.z * localNormal.z);

  // 3. Signed distance from OBB center to plane (using PlaneEquation)
  float d = PlaneEquation(obb->position);

  // 4. Return classification result:
  if (glm::abs(d) <= r) {
    return 0.0f; // Intersects plane
  } else if (d < -r) {
    return d + r; // Fully behind plane (outside frustum)
  }
  return d - r; // Fully in front of plane (inside frustum)
}
