#include "ComponentCollider.hpp"
#include "Graphics/Material.hpp"
#include "Physics/PhysicsSystem.hpp"
#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>
#include <limits>
#include <vector>

using namespace Engine::Physics;
using namespace Engine;

namespace {

struct OBB {
  glm::vec3 center;
  glm::vec3 halfExtents;
  glm::vec3 axes[3]; // Local rotated X, Y, Z axes
};

OBB GetOBB(const std::shared_ptr<Object> &obj,
           const std::vector<glm::vec3> &corners) {
  OBB obb;
  obb.center = obj->_position;

  glm::quat rot = glm::normalize(obj->_rotation);
  obb.axes[0] = glm::normalize(rot * glm::vec3(1.0f, 0.0f, 0.0f));
  obb.axes[1] = glm::normalize(rot * glm::vec3(0.0f, 1.0f, 0.0f));
  obb.axes[2] = glm::normalize(rot * glm::vec3(0.0f, 0.0f, 1.0f));

  if (!corners.empty()) {
    glm::vec3 minLocal = corners[0];
    glm::vec3 maxLocal = corners[0];
    for (size_t i = 1; i < corners.size(); ++i) {
      minLocal = glm::min(minLocal, corners[i]);
      maxLocal = glm::max(maxLocal, corners[i]);
    }
    obb.halfExtents = (maxLocal - minLocal) * 0.5f * obj->scale;
    glm::vec3 localCenter = (minLocal + maxLocal) * 0.5f * obj->scale;
    obb.center += rot * localCenter;
  } else {
    obb.halfExtents = glm::vec3(1.0f) * obj->scale;
  }

  return obb;
}

bool TestAxis(const glm::vec3 &axis, const OBB &a, const OBB &b,
              const glm::vec3 &d, float &outOverlap) {
  float lengthSq = glm::dot(axis, axis);
  if (lengthSq < 1e-6f) {
    outOverlap = std::numeric_limits<float>::max();
    return true;
  }

  glm::vec3 normalizedAxis = axis / glm::sqrt(lengthSq);

  float rA = a.halfExtents.x * glm::abs(glm::dot(normalizedAxis, a.axes[0])) +
             a.halfExtents.y * glm::abs(glm::dot(normalizedAxis, a.axes[1])) +
             a.halfExtents.z * glm::abs(glm::dot(normalizedAxis, a.axes[2]));

  float rB = b.halfExtents.x * glm::abs(glm::dot(normalizedAxis, b.axes[0])) +
             b.halfExtents.y * glm::abs(glm::dot(normalizedAxis, b.axes[1])) +
             b.halfExtents.z * glm::abs(glm::dot(normalizedAxis, b.axes[2]));

  float dist = glm::abs(glm::dot(d, normalizedAxis));

  outOverlap = (rA + rB) - dist;
  return outOverlap > 0.0f;
}

bool TestOBBCollision(const OBB &a, const OBB &b, glm::vec3 &outNormal,
                      float &outDepth) {
  glm::vec3 d = a.center - b.center;
  float minOverlap = std::numeric_limits<float>::max();
  glm::vec3 bestAxis(0.0f);

  std::vector<glm::vec3> axes = {a.axes[0],
                                 a.axes[1],
                                 a.axes[2],
                                 b.axes[0],
                                 b.axes[1],
                                 b.axes[2],
                                 glm::cross(a.axes[0], b.axes[0]),
                                 glm::cross(a.axes[0], b.axes[1]),
                                 glm::cross(a.axes[0], b.axes[2]),
                                 glm::cross(a.axes[1], b.axes[0]),
                                 glm::cross(a.axes[1], b.axes[1]),
                                 glm::cross(a.axes[1], b.axes[2]),
                                 glm::cross(a.axes[2], b.axes[0]),
                                 glm::cross(a.axes[2], b.axes[1]),
                                 glm::cross(a.axes[2], b.axes[2])};

  for (const auto &axis : axes) {
    float overlap = 0.0f;
    if (!TestAxis(axis, a, b, d, overlap)) {
      return false;
    }

    if (overlap < minOverlap) {
      minOverlap = overlap;
      bestAxis = glm::normalize(axis);
    }
  }

  if (glm::dot(d, bestAxis) < 0.0f) {
    bestAxis = -bestAxis;
  }

  outNormal = bestAxis;
  outDepth = minOverlap;
  return true;
}

} // anonymous namespace

void ComponentCollider::FromJson(json &js) {}

void Engine::Physics::ComponentCollider::OnCollisionEnter(
    std::shared_ptr<ComponentCollider> other) {
  if (func != nullptr) {
    func(other);
  }

  if (isTrigger || !other || other->isTrigger) {
    return;
  }

  auto selfObj = object.lock();
  auto otherObj = other->object.lock();
  if (!selfObj || !otherObj) {
    return;
  }

  auto cph = selfObj->GetComponent<ComponentPhysics>();
  if (cph == nullptr) {
    return;
  }

  OBB obbA = GetOBB(selfObj, this->Corners);
  OBB obbB = GetOBB(otherObj, other->Corners);

  glm::vec3 normal(0.0f);
  float depth = 0.0f;

  if (!TestOBBCollision(obbA, obbB, normal, depth)) {
    return;
  }

  glm::vec3 mtv = normal * depth;
  selfObj->_position += mtv;

  glm::dvec3 vel = cph->GetVelocities();
  double normalVel = glm::dot(vel, glm::dvec3(normal));
  if (normalVel < 0.0) {
    vel -= glm::dvec3(normal) * normalVel;
    cph->SetVelocities(vel);
  }
}

void Engine::Physics::ComponentCollider::Setup() {
  PhysicsSystem::RegisterCollider(this->shared_from_this());
}

void Engine::Physics::ComponentCollider::Update() { cachedCorners = {}; }

void Engine::Physics::ComponentCollider::End() {
  PhysicsSystem::UnregisterCollider(this->shared_from_this());
}

std::vector<glm::vec3> Engine::Physics::ComponentCollider::GetCachedCorners() {
  if (cachedCorners.empty()) {
    auto o = object.lock();
    if (o) {
      glm::quat rot = glm::normalize(o->_rotation);
      cachedCorners.reserve(this->Corners.size());
      for (const auto &corner : this->Corners) {
        cachedCorners.push_back((rot * (corner * o->scale)) + o->_position);
      }
    }
  }
  return cachedCorners;
}

bool Engine::Physics::ComponentCollider::DoesCollide(
    std::shared_ptr<ComponentCollider> other) {
  if (!other)
    return false;

  auto selfObj = object.lock();
  auto otherObj = other->object.lock();
  if (!selfObj || !otherObj)
    return false;

  OBB obbA = GetOBB(selfObj, this->Corners);
  OBB obbB = GetOBB(otherObj, other->Corners);

  glm::vec3 normal;
  float depth = 0.0f;
  return TestOBBCollision(obbA, obbB, normal, depth);
}

json ComponentCollider::ToJson() { return {}; }
