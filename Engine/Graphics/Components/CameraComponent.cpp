#include "CameraComponent.hpp"
#include "Engine.hpp"
#include "Interfaces/IComponent.hpp"
#include "Physics/Components/ComponentPhysics.hpp"
#include "Physics/PhysicsSystem.hpp"
#include "Scene.hpp"
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/trigonometric.hpp>
#include <memory>
#include <tracy/Tracy.hpp>
namespace Engine {
namespace Graphics {

// TODO:
using json = nlohmann::json;
void CameraComponent::EarlySetup() {
  ZoneScoped;
  auto obj = object.lock();
  if (!obj)
    return;
  recalculateMatrixes();
  w = Engine::Engine::width;
  h = Engine::Engine::height;
  oldrot = obj->GetRotation();
  oldpos = obj->GetPosition();
}

void CameraComponent::Update() {}

CameraComponent::CameraComponent(std::shared_ptr<Object> object) {
  ZoneScoped;
  this->object = object;
  this->near = 0.5f;
  this->far = 100.0f;
  // fromParams(name, comps, position, rotation);
}

CameraComponent::~CameraComponent() {}

void CameraComponent::FromJson(json &js) {
  CameraComponentJson ccjs = js;
  far = ccjs.far;
  near = ccjs.near;
}

json CameraComponent::ToJson() {
  CameraComponentJson ccjs;
  ccjs.far = far;
  ccjs.near = near;
  return ccjs;
}
CameraComponent::CameraComponent(json &js, std::shared_ptr<Object> object) {
  this->object = object;
  FromJson(js);
} //
glm::mat4 CameraComponent::GetProjMatrix() {
  if (!isUpToDate())
    recalculateMatrixes();
  return projmat;
}
glm::mat4 CameraComponent::GetViewMatrix() {
  if (!isUpToDate())
    recalculateMatrixes();
  return viewmat;
}

void CameraComponent::recalculateMatrixes() {
  ZoneScoped;
  auto obj = object.lock();
  if (!obj)
    return;
  if (Engine::Engine::width <= 0 || Engine::Engine::height <= 0)
    return;
  glm::vec3 frontnormal =
      glm::normalize(obj->GetRotation() * glm::vec3(0, 0, -1));
  this->front = frontnormal;
  this->right = glm::normalize(glm::cross(this->front, glm::vec3(0, 1, 0)));
  this->up = glm::normalize(glm::cross(this->right, this->front));
  projmat = glm::perspective(
      glm::radians(45.0f),
      (float)Engine::Engine::width / (float)Engine::Engine::height, near, far);

  // Default: use exact world position
  glm::vec3 camPos = obj->GetPosition();

  // If the camera is a child of an object with physics, smoothly lerp
  // its position using the parent's interpolated render position so that
  // it doesn't stutter between fixed physics ticks.
  if (auto so = obj->sceneobject.lock()) {
    if (auto parentSO = so->Parent.lock()) {
      if (parentSO->instance) {
        if (auto phys =
                parentSO->instance->GetComponent<Physics::ComponentPhysics>()) {
          glm::vec3 smoothParentPos = phys->GetRenderPosition();
          camPos = smoothParentPos + obj->GetLocalPosition();
        }
      }
    }
  }

  viewmat = glm::lookAt(camPos, camPos + frontnormal, glm::vec3(0, 1, 0));
  oldrot = obj->GetRotation();
  oldpos = obj->GetPosition();
  w = Engine::Engine::width;
  h = Engine::Engine::height;
}
bool CameraComponent::isUpToDate() {
  auto obj = object.lock();
  if (!obj)
    return false;

  // If the parent has physics and interpolation is on, the smooth position
  // changes every render frame — always recalculate.
  if (Physics::PhysicsSystem::interpolateRendering &&
      Engine::Engine::isSimulationEnabled) {
    if (auto so = obj->sceneobject.lock()) {
      if (auto parentSO = so->Parent.lock()) {
        if (parentSO->instance &&
            parentSO->instance->GetComponent<Physics::ComponentPhysics>()) {
          return false;
        }
      }
    }
  }

  return oldrot == obj->GetRotation() && oldpos == obj->GetPosition() &&
         w == Engine::Engine::width && h == Engine::Engine::height;
}

} // namespace Graphics
} // namespace Engine
