#include "CameraComponent.hpp"
#include "Engine.hpp"
#include "Interfaces/IComponent.hpp"
#include "Physics/Components/ComponentPhysics.hpp"
#include "Physics/PhysicsSystem.hpp"
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
glm::mat4 CameraComponent::GetFrustumMatrix() {
  if (w <= 0 || h <= 0 || near <= 0.0f || far <= near) {
    return glm::mat4(1.0f); // Return identity fallback to avoid NaNs
  }
  glm::mat4 viewProj = GetProjMatrix() * GetViewMatrix();
  float det = glm::determinant(viewProj);
  if (glm::abs(det) < 1e-6f) {
    return glm::mat4(1.0f);
  }
  return glm::inverse(viewProj);
}
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

  // Extract forward, right, and up directly from the Object's quaternion
  // rotation. This avoids cross products against world Up (0,1,0) and
  // eliminates Gimbal Lock singularities.
  glm::quat orientation = obj->GetRotation();

  this->front = glm::normalize(orientation * glm::vec3(0, 0, -1));
  this->right = glm::normalize(orientation * glm::vec3(1, 0, 0));
  this->up = glm::normalize(orientation * glm::vec3(0, 1, 0));

  projmat = glm::perspective(
      glm::radians(45.0f),
      (float)Engine::Engine::width / (float)Engine::Engine::height, near, far);

  glm::vec3 camPos = obj->GetPosition();

  // Handle physics interpolation if present
  if (auto so = obj->sceneobject.lock()) {
    if (auto parentSO = so->Parent.lock()) {
      if (parentSO->instance) {
        if (auto phys =
                parentSO->instance->GetComponent<Physics::ComponentPhysics>()) {
          glm::vec3 smoothParentPos = phys->GetRenderPosition();
          camPos = smoothParentPos +
                   parentSO->instance->GetRotation() * obj->GetLocalPosition();
        }
      }
    }
  }

  // Pass the camera's actual local UP vector instead of hardcoded world (0, 1,
  // 0)
  viewmat = glm::lookAt(camPos, camPos + this->front, this->up);

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

std::vector<glm::vec3> Frustum::GetCorners() {
  std::vector<glm::vec3> outCorners;
  outCorners.reserve(8);

  outCorners.push_back(
      planes.str.near.Intersection(planes.str.top, planes.str.left));
  outCorners.push_back(
      planes.str.near.Intersection(planes.str.top, planes.str.right));
  outCorners.push_back(
      planes.str.near.Intersection(planes.str.bottom, planes.str.left));
  outCorners.push_back(
      planes.str.near.Intersection(planes.str.bottom, planes.str.right));
  outCorners.push_back(
      planes.str.far.Intersection(planes.str.top, planes.str.left));
  outCorners.push_back(
      planes.str.far.Intersection(planes.str.top, planes.str.right));
  outCorners.push_back(
      planes.str.far.Intersection(planes.str.bottom, planes.str.left));
  outCorners.push_back(
      planes.str.far.Intersection(planes.str.bottom, planes.str.right));

  return outCorners;
}

} // namespace Graphics
} // namespace Engine
