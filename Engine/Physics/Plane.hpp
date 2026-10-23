#pragma once
#include "Physics/Ray.hpp"
#include "glm/glm.hpp"
#include <glm/matrix.hpp>
namespace Engine {
namespace Physics {
class Plane {
public:
  glm::vec3 normal;
  float distance;
  Plane() { normal = glm::vec3(1, 0, 0); }
  Plane(const glm::vec3 &n, float d) : normal(n), distance(d) {}
  float PlaneEquation(const glm::vec3 &pt) {
    return glm::dot(pt, normal) - distance;
  }

  glm::vec3 Intersection(Plane p2, Plane p3) {
    glm::mat3 D(normal.x, p2.normal.x, p3.normal.x, normal.y, p2.normal.y,
                p3.normal.y, normal.z, p2.normal.z, p3.normal.z);
    glm::vec3 A(-distance, -p2.distance, -p3.distance);
    glm::mat3 Dx = D;
    glm::mat3 Dy = D;
    glm::mat3 Dz = D;
    Dx[0][0] = A.x;
    Dx[0][1] = A.y;
    Dx[0][2] = A.z;
    Dy[1][0] = A.x;
    Dy[1][1] = A.y;
    Dy[1][2] = A.z;
    Dz[2][0] = A.x;
    Dz[2][1] = A.y;
    Dz[2][2] = A.z;
    float detD = glm::determinant(D);
    if (CMP(detD, 0)) {
      return glm::vec3();
    }
    float detDx = glm::determinant(Dx);
    float detDy = glm::determinant(Dy);
    float detDz = glm::determinant(Dz);
    return glm::vec3(detDx / detD, detDy / detD, detDz / detD);
  }
  float Classify(std::shared_ptr<OBB> obb);
};
} // namespace Physics
} // namespace Engine
