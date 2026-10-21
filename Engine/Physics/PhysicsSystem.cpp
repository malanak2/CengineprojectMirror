#include "PhysicsSystem.hpp"
#include "Components/ComponentPhysics.hpp"
#include "Engine.hpp"
#include "Physics/Components/ComponentBoundingBox.hpp"
#include "Util/LoggerUtil.hpp"
using namespace Engine::Physics;

glm::vec3 PhysicsSystem::world_direction = {0, -1, 0};
glm::vec3 PhysicsSystem::gravity_scale = {0.2, 1, 0.2};
float PhysicsSystem::physics_framerate = 60;
float PhysicsSystem::accumulator = 0;

std::vector<std::shared_ptr<ComponentPhysics>> PhysicsSystem::cphysics = {};
std::vector<std::shared_ptr<ComponentBoundingBox>>
    PhysicsSystem::boundingBoxes = {};
std::vector<std::pair<std::pair<std::shared_ptr<ComponentPhysics>,
                                std::shared_ptr<ComponentPhysics>>,
                      std::shared_ptr<CollisionManifold>>>
    PhysicsSystem::collisions = {};

float PhysicsSystem::LinearProjectionPercent = 0.45f;
float PhysicsSystem::PenetrationSlack = 0.01f;
int PhysicsSystem::ImpulseIteration = 7;
float PhysicsSystem::gravity_accel = 9.834f;

void Engine::Physics::PhysicsSystem::RegisterCPhysics(
    std::shared_ptr<ComponentPhysics> c) {
  cphysics.insert(cphysics.end(), c);
}

void Engine::Physics::PhysicsSystem::UnregisterCPhysics(
    std::shared_ptr<ComponentPhysics> c) {
  auto pos = std::find(cphysics.begin(), cphysics.end(), c);
  if (pos == cphysics.end()) {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                        "Tried to unregister nonexistent CPhysics");
    return;
  }
  cphysics.erase(pos);
}

void Engine::Physics::PhysicsSystem::RegisterBBox(
    std::shared_ptr<ComponentBoundingBox> c) {
  boundingBoxes.insert(boundingBoxes.end(), c);
}

void Engine::Physics::PhysicsSystem::UnregisterBBox(
    std::shared_ptr<ComponentBoundingBox> c) {
  auto pos = std::find(boundingBoxes.begin(), boundingBoxes.end(), c);
  if (pos == boundingBoxes.end()) {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                        "Tried to unregister nonexistent CPhysics");
    return;
  }
  boundingBoxes.erase(pos);
}
void PhysicsSystem::ApplyStaticImpulse(std::shared_ptr<ComponentPhysics> body,
                                       std::shared_ptr<CollisionManifold> M,
                                       int) {
  float invMass = (body->mass > 0.0f) ? 1.0f / body->mass : 0.0f;
  if (invMass == 0.0f)
    return;

  glm::vec3 v = body->GetVelocities();
  glm::vec3 n = glm::normalize(M->normal);

  float vn = glm::dot(v, n);
  if (vn <= 0.0f)
    return;

  float e = body->cor;
  if (vn < 0.35f)
    e = 0.0f;

  if (e == 0.0f && vn < 0.1f) {
    body->SetVelocities(v - n * vn);
    return;
  }
  body->SetVelocities(v - n * (1.0f + e) * vn);
}

void Engine::Physics::PhysicsSystem::ProcessPhysics() {
  accumulator += std::min(Engine::GetDeltaTime(), 0.25f);
  float physdelta = 1 / physics_framerate;
  while (accumulator >= physdelta) {
    ZoneScoped;
    phystick();
    accumulator -= physdelta;
  }
}
void Engine::Physics::PhysicsSystem::phystick() {
  ZoneScoped;
  collisions.clear();

  for (size_t i = 0, size = cphysics.size(); i < size; ++i) {
    cphysics[i]->UpdatePhysics();
  }

  for (size_t i = 0, cSize = cphysics.size(); i < cSize; ++i) {
    auto physObj = cphysics[i]->object.lock();
    if (!physObj)
      continue;

    auto physBBoxComp = physObj->GetComponent<ComponentBoundingBox>();
    if (!physBBoxComp)
      continue;

    auto m1 = physBBoxComp->GetOBB();
    if (!m1)
      continue;

    for (size_t j = 0, bSize = boundingBoxes.size(); j < bSize; ++j) {
      if (boundingBoxes[j] == physBBoxComp)
        continue;

      auto m2 = boundingBoxes[j]->GetOBB();
      if (!m2)
        continue;

      auto result = m1->FindCollisionFeatures(m2);

      if (result && result->colliding) {
        std::shared_ptr<ComponentPhysics> otherPhysics = nullptr;
        auto otherObj = boundingBoxes[j]->object.lock();
        if (otherObj) {
          otherPhysics = otherObj->GetComponent<ComponentPhysics>();
        }

        collisions.push_back(
            std::make_pair(std::make_pair(cphysics[i], otherPhysics), result));
      }
    }
  }

  for (int k = 0; k < ImpulseIteration; ++k) {
    for (size_t i = 0; i < collisions.size(); ++i) {
      auto &p = collisions[i];
      auto m1 = p.first.first;
      auto m2 = p.first.second;
      auto manifold = p.second;

      if (m2) {
        m1->ApplyImpulse(m2, manifold, 0);
      } else {
        ApplyStaticImpulse(m1, manifold, 0);
      }
    }
  }

  for (size_t i = 0, size = cphysics.size(); i < size; ++i) {
    cphysics[i]->ApplyForces();
  }
  for (size_t i = 0, size = collisions.size(); i < size; ++i) {
    auto &p = collisions[i];
    auto m1 = p.first.first;
    auto m2 = p.first.second;
    auto manifold = p.second;

    float invMass1 = (m1->mass > 0.0f) ? (1.0f / m1->mass) : 0.0f;
    float invMass2 = (m2 && m2->mass > 0.0f) ? (1.0f / m2->mass) : 0.0f;
    float totalInvMass = invMass1 + invMass2;
    if (totalInvMass == 0.0f)
      continue;

    glm::vec3 norm = glm::normalize(manifold->normal); // points m1 -> m2/static

    constexpr float SLACK = 0.001f;
    constexpr float PERCENT = 0.6f;

    float depth = std::max(manifold->depth - SLACK, 0.0f);
    if (depth <= 0.0f)
      continue;

    glm::vec3 correction = norm * (depth / totalInvMass) * PERCENT;

    auto om1 = m1->object.lock();
    if (om1 && invMass1 > 0.0f) {
      om1->_position -= correction * invMass1;
      glm::vec3 v1 = m1->GetVelocities();
      float vNorm = glm::dot(v1, norm);
      if (vNorm > 0.0f)
        m1->SetVelocities(v1 - norm * vNorm);
      if (auto bbox = om1->GetComponent<ComponentBoundingBox>())
        bbox->InvalidateBB();
    }

    if (m2) {
      auto om2 = m2->object.lock();
      if (om2 && invMass2 > 0.0f) {
        om2->_position += correction * invMass2;
        if (auto bbox = om2->GetComponent<ComponentBoundingBox>())
          bbox->InvalidateBB();
      }
    }
  }
}
