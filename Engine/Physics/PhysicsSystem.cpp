#include "PhysicsSystem.hpp"
#include "Components/ComponentCollider.hpp"
#include "Components/ComponentPhysics.hpp"
using namespace Engine::Physics;

glm::dvec3 PhysicsSystem::world_direction = {0, -1, 0};
glm::dvec3 PhysicsSystem::gravity_scale = {0.2, 1, 0.2};

std::vector<std::shared_ptr<ComponentCollider>> PhysicsSystem::colliders = {};
std::vector<std::shared_ptr<ComponentPhysics>> PhysicsSystem::cphysics = {};

double PhysicsSystem::gravity_accel = 9.834;

void Engine::Physics::PhysicsSystem::RegisterCollider(
    std::shared_ptr<ComponentCollider> c) {
  colliders.insert(colliders.end(), c);
}
void Engine::Physics::PhysicsSystem::UnregisterCollider(
    std::shared_ptr<ComponentCollider> c) {
  colliders.erase(std::find(colliders.begin(), colliders.end(), c));
}
void Engine::Physics::PhysicsSystem::RegisterCPhysics(
    std::shared_ptr<ComponentPhysics> c) {
  cphysics.insert(cphysics.end(), c);
}
void Engine::Physics::PhysicsSystem::UnregisterCPhysics(
    std::shared_ptr<ComponentPhysics> c) {
  cphysics.erase(std::find(cphysics.begin(), cphysics.end(), c));
}
void Engine::Physics::PhysicsSystem::ProcessPhysics() {
  for (auto coltwo : colliders) {
    for (auto c : cphysics) {
      auto o = c->object.lock();
      auto col = o->GetComponent<ComponentCollider>();
      if (o == nullptr)
        continue;
      if (col != coltwo) {
        if (col->DoesCollide(coltwo)) {
          o->_position -= c->GetVelocities();
        }
      }
    }
  }
}
