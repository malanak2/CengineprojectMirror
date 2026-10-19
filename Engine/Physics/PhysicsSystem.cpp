#include "PhysicsSystem.hpp"
#include "Components/ComponentPhysics.hpp"
#include "Physics/Line.hpp"
#include "Util/LoggerUtil.hpp"
using namespace Engine::Physics;

glm::vec3 PhysicsSystem::world_direction = {0, -1, 0};
glm::vec3 PhysicsSystem::gravity_scale = {0.2, 1, 0.2};

std::vector<std::shared_ptr<ComponentPhysics>> PhysicsSystem::cphysics = {};

std::vector<std::shared_ptr<ComponentBoundingBox>>
    PhysicsSystem::boundingBoxes = {};

float PhysicsSystem::gravity_accel = 9.834;

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
void Engine::Physics::PhysicsSystem::ProcessPhysics() {
  for (auto phys : cphysics) {
    auto o = phys->object.lock();
    auto bb = o->GetComponent<ComponentBoundingBox>();
    Line traveled(phys->oldPos, o->_position);
    for (auto bbo : boundingBoxes) {
      if (bbo == bb)
        continue;
      if (traveled.Linetest(bbo->GetOBB())) {
        glm::vec3 direction = glm::normalize(phys->GetVelocities());
        Ray ray(phys->oldPos, direction);
        auto res = ray.Intersects(bbo->GetOBB());
        if (res.hit) {
          o->_position = res.point + res.normal * 0.002f;
          glm::vec3 vn =
              res.normal * glm::dot(res.normal, phys->GetVelocities());
          glm::vec3 vt = phys->GetVelocities() - vn;
          phys->oldPos = o->_position;
          phys->SetVelocities(vt - vn * phys->bounce);
        }
      }
    }
  }
}
