#include "ComponentPhysics.hpp"
#include "Engine.hpp"
#include "Object.hpp"
#include "Physics/PhysicsSystem.hpp"
#include <glm/vector_relational.hpp>

using namespace Engine::Physics;
using namespace Engine;

/*json ComponentPhysics::ToJson() {
  return {
      {"gravity_scale", {gravity_scale.x, gravity_scale.y, gravity_scale.z}},
      {"world_direction",
       {world_direction.x, world_direction.y, world_direction.z}},
      {"velocities", {velocities.x, velocities.y, velocities.z}},
      {"cor", cor},
      {"mass", mass},
      {"friction", friction}};
}*/
void Physics::ComponentPhysics::BeginTick() {
  if (auto o = object.lock())
    prevPos = o->GetPosition();
}

void Physics::ComponentPhysics::EndTick() {
  if (auto o = object.lock()) {
    currPos = o->GetPosition();
    hasTickState = true;
  }
}

glm::vec3 Physics::ComponentPhysics::GetRenderPosition() {
  auto o = object.lock();
  if (!o)
    return glm::vec3(0.0f);
  if (!hasTickState || !PhysicsSystem::interpolateRendering ||
      !Engine::Engine::isSimulationEnabled)
    return o->GetPosition();
  if (o->GetPosition() != currPos)
    return o->GetPosition();
  return glm::mix(prevPos, currPos, PhysicsSystem::GetInterpolationAlpha());
}
/*void ComponentPhysics::FromJson(json &js) {
  gravity_scale.x = js["gravity_scale"][0].get<float>();
  gravity_scale.y = js["gravity_scale"][1].get<float>();
  gravity_scale.z = js["gravity_scale"][2].get<float>();
  world_direction.x = js["world_direction"][0].get<float>();
  world_direction.y = js["world_direction"][1].get<float>();
  world_direction.z = js["world_direction"][2].get<float>();
  velocities.x = js["velocities"][0].get<float>();
  velocities.y = js["velocities"][1].get<float>();
  velocities.z = js["velocities"][2].get<float>();
  cor = js["cor"].get<float>();
  mass = js["mass"].get<float>();
  friction = js["friction"].get<float>();
}*/
void ComponentPhysics::UpdatePhysics() {
  if (!processPhysics)
    return;

  auto wrld_dir =
      useGlobalDirection ? PhysicsSystem::world_direction : world_direction;
  auto grv_scl =
      useGlobalGravity ? PhysicsSystem::gravity_scale : gravity_scale;

  glm::vec3 gravityAccel = wrld_dir * grv_scl * PhysicsSystem::gravity_accel;
  velocities += gravityAccel * GetDeltaTime();
  if (glm::length2(velocities) < 0.0009f) {
    velocities = glm::vec3(0.0f);
  }
}

void Physics::ComponentPhysics::EarlySetup() {
  PhysicsSystem::RegisterCPhysics(this->shared_from_this());
  world_direction = PhysicsSystem::world_direction;
  gravity_scale = PhysicsSystem::gravity_scale;
}

void Physics::ComponentPhysics::LateEnd() {
  PhysicsSystem::UnregisterCPhysics(this->shared_from_this());
}

void Physics::ComponentPhysics::ApplyForces() {
  auto o = object.lock();
  if (!o)
    return;

  oldPos = o->GetPosition();
  o->AddPosition(velocities * GetDeltaTime());
  auto obb = o->GetComponent<ComponentBoundingBox>();
  obb->InvalidateBB();
}
void Physics::ComponentPhysics::ApplyImpulse(
    const std::shared_ptr<ComponentPhysics> &B,
    const std::shared_ptr<CollisionManifold> &M) {

  float invMass1 = (mass > 0.0f) ? (1.0f / mass) : 0.0f;
  float invMass2 = (B && B->mass > 0.0f) ? (1.0f / B->mass) : 0.0f;
  float invMassSum = invMass1 + invMass2;

  if (invMassSum == 0.0f)
    return;

  glm::vec3 relativeVel =
      (B ? B->GetVelocities() : glm::vec3(0.0f)) - GetVelocities();
  glm::vec3 relativeNorm = glm::normalize(M->normal);

  float velAlongNormal = glm::dot(relativeVel, relativeNorm);

  if (velAlongNormal > 0.0f)
    return;

  float e = B ? std::min(cor, B->cor) : cor;

  const float minBounceVel = 0.55f;
  if (std::abs(velAlongNormal) < minBounceVel) {
    e = 0.0f;
  }

  if (e == 0.0f && std::abs(velAlongNormal) < 0.1f) {
    glm::vec3 v1 = GetVelocities();
    glm::vec3 normVel = relativeNorm * glm::dot(v1, relativeNorm);
    SetVelocities(v1 - normVel);
    return;
  }

  float j = -(1.0f + e) * velAlongNormal / invMassSum;
  glm::vec3 impulse = relativeNorm * j;

  this->Impulse(-impulse * invMass1);
  if (B && invMass2 > 0.0f) {
    B->Impulse(impulse * invMass2);
  }

  // Tangential / Friction Impulse
  relativeVel = (B ? B->GetVelocities() : glm::vec3(0.0f)) - GetVelocities();
  glm::vec3 tangent =
      relativeVel - (relativeNorm * glm::dot(relativeVel, relativeNorm));
  float tangentSqLen = glm::dot(tangent, tangent);

  if (tangentSqLen > 1e-6f) {
    tangent = glm::normalize(tangent);
    float jt = -glm::dot(relativeVel, tangent) / invMassSum;

    float frictionCoeff =
        B ? std::sqrt(this->friction * B->friction) : this->friction;
    float maxFriction = j * frictionCoeff;
    jt = std::clamp(jt, -maxFriction, maxFriction);

    glm::vec3 tangentImpulse = tangent * jt;
    this->Impulse(-tangentImpulse * invMass1);
    if (B && invMass2 > 0.0f) {
      B->Impulse(tangentImpulse * invMass2);
    }
  }
}
float Physics::ComponentPhysics::GetDeltaTime() {
  return 1 / PhysicsSystem::physics_framerate;
}

void Physics::CollisionManifold::ResetCollisionManifold() {
  colliding = false;
  normal = glm::vec3(0, 0, 1);
  depth = FLT_MAX;
  contacts.clear();
}
