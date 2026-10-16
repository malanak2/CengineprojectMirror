#include "PhysicsSystem.hpp"
using namespace Engine::Physics;

glm::dvec3 PhysicsSystem::world_direction = {0, -1, 0};
glm::dvec3 PhysicsSystem::gravity_scale = {0.2, 1, 0.2};
double PhysicsSystem::gravity_accel = 9.834;
