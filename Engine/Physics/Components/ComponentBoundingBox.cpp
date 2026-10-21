#include "ComponentBoundingBox.hpp"
#include "Physics/Components/ComponentPhysics.hpp"
#include "Physics/Line.hpp"
#include "Physics/PhysicsSystem.hpp"
#include "Physics/Plane.hpp"
#include "Physics/Ray.hpp"
std::pair<double, double> Engine::Physics::OBB::GetInterval(glm::vec3 axis) {
  glm::vec3 vertex[8];
  glm::vec3 C = this->position; // OBB Center
  glm::vec3 E = this->size;     // OBB Extents
  auto o1 = glm::transpose(this->orientation);

  glm::vec3 A[] = {o1[0], o1[1], o1[2]};
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
    for (int j = 0; j < 3; ++j) {
      test[6 + i * 3 + j] = glm::cross(test[i], test[3 + j]);
    }
  }
  for (int i = 0; i < 15; ++i) {
    if (!OverlapOnAxis(this->shared_from_this(), other, test[i])) {
      return false; // Seperating axis found
    }
  }
  return true; // Seperating axis not found
}
std::vector<glm::vec3> Engine::Physics::OBB::GetVertices() {
  std::vector<glm::vec3> v;
  v.resize(8);
  glm::vec3 C = position; // OBB Center
  glm::vec3 E = size;     // OBB Extents
  auto o = glm::transpose(orientation);

  glm::vec3 A[] = {o[0], o[1], o[2]};

  v[0] = C + A[0] * E[0] + A[1] * E[1] + A[2] * E[2];
  v[1] = C - A[0] * E[0] + A[1] * E[1] + A[2] * E[2];
  v[2] = C + A[0] * E[0] - A[1] * E[1] + A[2] * E[2];
  v[3] = C + A[0] * E[0] + A[1] * E[1] - A[2] * E[2];
  v[4] = C - A[0] * E[0] - A[1] * E[1] - A[2] * E[2];
  v[5] = C + A[0] * E[0] - A[1] * E[1] - A[2] * E[2];
  v[6] = C - A[0] * E[0] + A[1] * E[1] - A[2] * E[2];
  v[7] = C - A[0] * E[0] - A[1] * E[1] + A[2] * E[2];
  return v;
}
std::vector<Engine::Physics::Line> Engine::Physics::OBB::GetEdges() {
  std::vector<Line> result;
  result.reserve(12);
  std::vector<glm::vec3> v = GetVertices();
  int index[][2] = {// Indices of edge-vertices
                    {6, 1}, {6, 3}, {6, 4}, {2, 7}, {2, 5}, {2, 0},
                    {0, 1}, {0, 3}, {7, 1}, {7, 4}, {4, 5}, {5, 3}};
  for (int j = 0; j < 12; ++j) {
    result.push_back(Line(v[index[j][0]], v[index[j][1]]));
  }
  return result;
}
std::vector<Engine::Physics::Plane> Engine::Physics::OBB::GetPlanes() {

  glm::vec3 c = position;
  glm::vec3 e = size;
  auto o = glm::transpose(orientation);

  glm::vec3 a[] = {o[0], o[1], o[2]};
  std::vector<Plane> result;
  result.resize(6);
  result[0] = Plane(a[0], glm::dot(a[0], (c + a[0] * e.x)));
  result[1] = Plane(a[0] * -1.0f, -glm::dot(a[0], (c - a[0] * e.x)));
  result[2] = Plane(a[1], glm::dot(a[1], (c + a[1] * e.y)));
  result[3] = Plane(a[1] * -1.0f, -glm::dot(a[1], (c - a[1] * e.y)));
  result[4] = Plane(a[2], glm::dot(a[2], (c + a[2] * e.z)));
  result[5] = Plane(a[2] * -1.0f, -glm::dot(a[2], (c - a[2] * e.z)));
  return result;
}
std::pair<bool, glm::vec3> Engine::Physics::OBB::ClipToPlane(Plane plane,
                                                             Line line) {
  glm::vec3 ab = line.end - line.start;
  float nAB = glm::dot(plane.normal, ab);
  if (CMP(nAB, 0)) {
    return std::make_pair<bool, glm::vec3>(false, glm::vec3());
  }
  float nA = glm::dot(plane.normal, line.start);
  float t = (plane.distance - nA) / nAB;
  if (t >= 0.0f && t <= 1.0f) {
    return std::make_pair<bool, glm::vec3>(true, line.start + ab * t);
  }
  return std::make_pair<bool, glm::vec3>(false, glm::vec3());
}
std::vector<glm::vec3>
Engine::Physics::OBB::ClipEdgesToOBB(std::vector<Line> edges) {
  std::vector<glm::vec3> result;
  result.reserve(edges.size());
  const std::vector<Plane> &planes = GetPlanes();
  for (int i = 0; i < planes.size(); ++i) {
    for (int j = 0; j < edges.size(); ++j) {
      auto res = ClipToPlane(planes[i], edges[j]);
      if (res.first) {
        if (PointInOBB(res.second)) {
          result.push_back(res.second);
        }
      }
    }
  }
  return result;
}
std::pair<bool, float>
Engine::Physics::OBB::PenetrationDepth(std::shared_ptr<OBB> o2,
                                       glm::vec3 axis) {

  auto i1 = GetInterval(glm::normalize(axis));
  auto i2 = o2->GetInterval(glm::normalize(axis));
  if (!((i2.first <= i1.second) && (i1.first <= i2.second))) {
    return std::pair<bool, float>(false, 0.0f);
  }
  float len1 = i1.second - i1.first;
  float len2 = i2.second - i2.first;
  float min = std::min(i1.first, i2.first);
  float max = std::max(i1.second, i2.second);
  float length = max - min;
  return std::make_pair<bool, float>(i2.first < i1.first,
                                     (len1 + len2) - length);
}
std::shared_ptr<Engine::Physics::CollisionManifold>
Engine::Physics::OBB::FindCollisionFeatures(std::shared_ptr<OBB> B) {

  auto result = std::make_shared<CollisionManifold>();
  result->ResetCollisionManifold();

  auto o1 = orientation;
  auto o2 = B->orientation;

  std::vector<glm::vec3> test(15);
  test[0] = glm::vec3(o1[0][0], o1[0][1], o1[0][2]);
  test[1] = glm::vec3(o1[1][0], o1[1][1], o1[1][2]);
  test[2] = glm::vec3(o1[2][0], o1[2][1], o1[2][2]);

  test[3] = glm::vec3(o2[0][0], o2[0][1], o2[0][2]);
  test[4] = glm::vec3(o2[1][0], o2[1][1], o2[1][2]);
  test[5] = glm::vec3(o2[2][0], o2[2][1], o2[2][2]);

  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      test[6 + i * 3 + j] = glm::cross(test[i], test[3 + j]);
    }
  }

  result->depth = std::numeric_limits<float>::max();
  int bestAxis = -1;
  glm::vec3 bestNormalizedAxis(0.0f);

  for (int i = 0; i < 15; ++i) {
    float lenSq = glm::dot(test[i], test[i]);
    if (lenSq < 0.0001f)
      continue; // Skip parallel edges

    glm::vec3 axis = test[i] / std::sqrt(lenSq);
    auto r = PenetrationDepth(B, axis);
    float depth = r.second;

    if (depth <= 0.0f) {
      return result;
    }

    if (depth < result->depth) {
      result->depth = depth;
      bestAxis = i;
      bestNormalizedAxis = r.first ? -axis : axis;
    }
  }

  if (bestAxis == -1)
    return result;

  glm::vec3 axis = bestNormalizedAxis;

  // Contact manifold generation via edge clipping
  std::vector<glm::vec3> c1 = ClipEdgesToOBB(B->GetEdges());
  std::vector<glm::vec3> c2 = ClipEdgesToOBB(GetEdges());
  result->contacts.reserve(c1.size() + c2.size());
  result->contacts.insert(result->contacts.end(), c1.begin(), c1.end());
  result->contacts.insert(result->contacts.end(), c2.begin(), c2.end());

  auto interval = GetInterval(axis);
  float distance =
      (interval.second - interval.first) * 0.5f - result->depth * 0.5f;
  glm::vec3 pointOnPlane = position + axis * distance;

  for (int i = static_cast<int>(result->contacts.size()) - 1; i >= 0; --i) {
    glm::vec3 contact = result->contacts[i];
    result->contacts[i] =
        contact + (axis * glm::dot(axis, pointOnPlane - contact));
    for (int j = static_cast<int>(result->contacts.size()) - 1; j > i; --j) {
      if (glm::dot(result->contacts[j] - result->contacts[i],
                   result->contacts[j] - result->contacts[i]) < 0.0001f) {
        result->contacts.erase(result->contacts.begin() + j);
        break;
      }
    }
  }

  result->colliding = true;
  result->normal = axis;
  return result;
}

bool Engine::Physics::OBB::PointInOBB(glm::vec3 point) {
  glm::vec3 dir = point - position;
  auto ori = glm::transpose(orientation);

  for (int i = 0; i < 3; ++i) {
    glm::vec3 axis = glm::vec3(ori[i][0], ori[i][1], ori[i][2]);
    float distance = glm::dot(dir, axis);
    if (std::abs(distance) > size[i]) {
      return false;
    }
  }
  return true;
}

void Engine::Physics::ComponentBoundingBox::Setup() {
  PhysicsSystem::RegisterBBox(this->shared_from_this());
}
void Engine::Physics::ComponentBoundingBox::End() {
  PhysicsSystem::UnregisterBBox(this->shared_from_this());
}
void Engine::Physics::ComponentBoundingBox::Update() { InvalidateBB(); }

std::shared_ptr<Engine::Physics::OBB>
Engine::Physics::ComponentBoundingBox::GetOBB() {
  if (obb != nullptr)
    return obb;
  auto o = object.lock();
  auto obb = std::make_shared<OBB>();
  obb->position = o->_position + o->_rotation * position_offset;
  obb->size = size;
  obb->orientation =
      glm::toMat3(glm::normalize(o->_rotation * orientation_offset));
  this->obb = obb;
  return obb;
}
