#include "ComponentPhysics.hpp"
#include "Object.hpp"
#include "Physics/PhysicsSystem.hpp"
#include <glm/vector_relational.hpp>

using namespace Engine::Physics;
using namespace Engine;
json ComponentPhysics::ToJson() { return {}; }
void ComponentPhysics::FromJson(json &js) {}
void ComponentPhysics::Update() {
  if (velocities == (world_direction * gravity_scale))
    return;
  for (int i = 0; i < velocities.length(); i++) {
    auto target =
        (world_direction * gravity_scale * PhysicsSystem::gravity_accel);
    auto target_set =
        PhysicsSystem::gravity_accel * GetDeltaTime() * gravity_scale;
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
  }
}
