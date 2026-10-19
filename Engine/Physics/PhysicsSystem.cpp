#include "PhysicsSystem.hpp"
#include "Components/ComponentPhysics.hpp"
#include "Util/LoggerUtil.hpp"
using namespace Engine::Physics;

glm::dvec3 PhysicsSystem::world_direction = {0, -1, 0};
glm::dvec3 PhysicsSystem::gravity_scale = {0.2, 1, 0.2};

std::vector<std::shared_ptr<ComponentPhysics>> PhysicsSystem::cphysics = {};

double PhysicsSystem::gravity_accel = 9.834;

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

void Engine::Physics::PhysicsSystem::ProcessPhysics() {}
