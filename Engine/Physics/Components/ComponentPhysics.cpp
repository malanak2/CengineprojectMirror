#include "ComponentPhysics.hpp"
#include "Object.hpp"
#include "Physics/PhysicsSystem.hpp"
#include <glm/vector_relational.hpp>

using namespace Engine::Physics;
using namespace Engine;
json ComponentPhysics::ToJson() {
  return {
      {"gravity_scale", {gravity_scale.x, gravity_scale.y, gravity_scale.z}},
      {"world_direction",
       {world_direction.x, world_direction.y, world_direction.z}},
      {"velicities", {velocities.x, velocities.y, velocities.z}}};
}
void ComponentPhysics::FromJson(json &js) {
  gravity_scale.x = js["gravity_scale"][0].get<double>();
  gravity_scale.y = js["gravity_scale"][1].get<double>();
  gravity_scale.z = js["gravity_scale"][2].get<double>();
  world_direction.x = js["world_direction"][0].get<double>();
  world_direction.y = js["world_direction"][1].get<double>();
  world_direction.z = js["world_direction"][2].get<double>();
  velocities.x = js["velocities"][0].get<double>();
  velocities.y = js["velocities"][1].get<double>();
  velocities.z = js["velocities"][2].get<double>();
}
void ComponentPhysics::Update() {
  if (velocities == (world_direction * gravity_scale))
    return;
  for (int i = 0; i < velocities.length(); i++) {
    auto target =
        (world_direction * gravity_scale * PhysicsSystem::gravity_accel);
    auto target_set =
        PhysicsSystem::gravity_accel * GetDeltaTime() * gravity_scale[i];
    if (velocities[i] < target[i]) {
      velocities[i] += target_set;
      if (velocities[i] > target[i]) {
        velocities[i] = target[i];
      }
    } else {
      velocities[i] -= target_set;
      if (velocities[i] < target[i]) {
        velocities[i] = target[i];
      }
    }
    auto o = object.lock();
    o->_position += velocities * (double)GetDeltaTime();

    if (o->_position.y > 0) {
      world_direction = {0, -1, 0};
    } else if (o->_position.y < 0) {
      world_direction = {0, 1, 0};
    }
  }
}
void Engine::Physics::ComponentPhysics::Setup() {
  velocities = {0, 4, 1};
  gravity_scale = {0, 1, 0};
}
