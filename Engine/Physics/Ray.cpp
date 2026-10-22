#include "Ray.hpp"
#include "Physics/Components/ComponentBoundingBox.hpp"
#include <glm/geometric.hpp>
Engine::Physics::RaycastResult

Engine::Physics::Ray::Intersects(std::shared_ptr<OBB> obb) {
  auto o = glm::transpose(obb->orientation);
  auto size = obb->size;
  // X, Y and Z axis of OBB
  glm::vec3 X(o[0][0], o[0][1], o[0][2]);
  glm::vec3 Y(o[1][0], o[1][1], o[1][2]);
  glm::vec3 Z(o[2][0], o[2][1], o[2][2]);
  glm::vec3 p = obb->position - origin;
  glm::vec3 f(glm::dot(X, direction), glm::dot(Y, direction),
              glm::dot(Z, direction));
  glm::vec3 e(glm::dot(X, p), glm::dot(Y, p), glm::dot(Z, p));
  float t[6] = {0, 0, 0, 0, 0, 0};
  RaycastResult outResult;
  outResult.hit = false;
  outResult.t = -1;
  for (int i = 0; i < 3; ++i) {
    if (CMP(f[i], 0)) {
      if (-e[i] - size[i] > 0 || -e[i] + size[i] < 0) {
        return outResult;
      }
      f[i] = 0.00001f; // Avoid div by 0!
    }
    t[i * 2 + 0] = (e[i] + size[i]) / f[i]; // min
    t[i * 2 + 1] = (e[i] - size[i]) / f[i]; // max
  }
  float tmin = std::max(std::max(std::min(t[0], t[1]), std::min(t[2], t[3])),
                        std::min(t[4], t[5]));

  float tmax =
      fminf(fminf(fmaxf(t[0], t[1]), fmaxf(t[2], t[3])), fmaxf(t[4], t[5]));
  if (tmax < 0) {

    return outResult;
  }

  if (tmin > tmax) {
    return outResult;
  }

  outResult.hit = true;
  if (tmin < 0.0f) {

    outResult.t = tmax;
    outResult.point = origin + direction * tmax;
    outResult.normal = glm::normalize(outResult.point - obb->position);
    return outResult;
  }
  outResult.t = tmin;
  outResult.point = origin + direction * tmin;
  outResult.normal = glm::normalize(outResult.point - obb->position);
  return outResult;
}
