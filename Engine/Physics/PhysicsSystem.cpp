#include "PhysicsSystem.hpp"
using namespace Engine::Physics;

glm::dvec3 PhysicsSystem::world_direction = {0, -1, 0};
double PhysicsSystem::gravity_scale = 1;
double PhysicsSystem::gravity_accel = 9.834;
