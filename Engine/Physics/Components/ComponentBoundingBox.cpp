#include "ComponentBoundingBox.hpp"
std::pair<double, double> Engine::Physics::OBB::GetInterval(glm::vec3 axis) {
  glm::vec3 vertex[8];
  glm::vec3 C = this->position; // OBB Center
  glm::vec3 E = this->size;     // OBB Extents
  auto o1 = this->orientation;
  glm::vec3 A[] = {glm::vec3(o1[0][0], o1[0][1], o1[0][2]),
                   glm::vec3(o1[1][0], o1[1][1], o1[1][2]),
                   glm::vec3(o1[2][0], o1[2][1], o1[2][2])};
  // OBB Axis
  vertex[0] = C + A[0] * E[0] + A[1] * E[1] + A[2] * E[2];
  vertex[1] = C - A[0] * E[0] + A[1] * E[1] + A[2] * E[2];
  vertex[2] = C + A[0] * E[0] - A[1] * E[1] + A[2] * E[2];
  vertex[3] = C + A[0] * E[0] + A[1] * E[1] - A[2] * E[2];
  vertex[4] = C - A[0] * E[0] - A[1] * E[1] - A[2] * E[2];
  vertex[5] = C + A[0] * E[0] - A[1] * E[1] - A[2] * E[2];
  vertex[6] = C - A[0] * E[0] + A[1] * E[1] - A[2] * E[2];
  vertex[7] = C - A[0] * E[0] - A[1] * E[1] + A[2] * E[2];
  std::pair<double, double> result;
  result.first = result.second = glm::dot(axis, vertex[0]);
  for (int i = 1; i < 8; ++i) {
    float projection = glm::dot(axis, vertex[i]);
    result.first = (projection < result.first) ? projection : result.first;
    result.second = (projection > result.second) ? projection : result.second;
  }
  return result;
}
bool Engine::Physics::OBB::OverlapOnAxis(std::shared_ptr<OBB> obb1,
                                         std::shared_ptr<OBB> obb2,
                                         const glm::vec3 &axis) {
  auto a = GetInterval(axis);
  auto b = obb2->GetInterval(axis);
  return ((b.first <= a.second) && (a.first <= b.second));
}
bool Engine::Physics::OBB::Collides(std::shared_ptr<OBB> other) {
  auto o1 = this->orientation;
  auto o2 = other->orientation;
  std::vector<glm::vec3> test = {glm::vec3(o1[0][0], o1[0][1], o1[0][2]),
                                 glm::vec3(o1[1][0], o1[1][1], o1[1][2]),
                                 glm::vec3(o1[2][0], o1[2][1], o1[2][2]),
                                 glm::vec3(o2[0][0], o2[0][1], o2[0][2]),
                                 glm::vec3(o2[1][0], o2[1][1], o2[1][2]),
                                 glm::vec3(o2[2][0], o2[2][1], o2[2][2]),
                                 {},
                                 {},
                                 {},
                                 {},
                                 {},
                                 {},
                                 {},
                                 {},
                                 {},
                                 {}};
  for (int i = 0; i < 3; ++i) {
    test[6 + i * 3 + 0] = glm::cross(test[i], test[0]);
    test[6 + i * 3 + 1] = glm::cross(test[i], test[1]);
    test[6 + i * 3 + 2] = glm::cross(test[i], test[2]);
  }
  for (int i = 0; i < 15; ++i) {
    if (!OverlapOnAxis(this->shared_from_this(), other, test[i])) {
      return false; // Seperating axis found
    }
  }
  return true; // Seperating axis not found
};
std::shared_ptr<Engine::Physics::OBB>
Engine::Physics::ComponentBoundingBox::GetOBB() {
  auto o = object.lock();
  auto obb = std::make_shared<OBB>();
  obb->position = o->_position;
  obb->size = glm::vec3(o->scale, o->scale, o->scale);
  obb->orientation = glm::transpose(glm::toMat3(o->_rotation));
  return obb;
}
