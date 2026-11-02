#include "ComponentSocket.hpp"
#include "Graphics/Components/ComponentAnimator.hpp"
#include "Object.hpp"
#include "Scene.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

void Engine::Graphics::ComponentSocket::Update() {
  auto owner = object.lock();
  if (!owner || target_object.empty() || target_bone.empty())
    return;

  auto sc = owner->scene.lock();
  if (!sc)
    return;

  auto targetObj = sc->FindObject(target_object);
  if (!targetObj)
    return;

  auto animator = targetObj->GetComponent<ComponentAnimator>();
  if (!animator)
    return;

  glm::mat4 boneWorld = animator->GetBoneWorldTransform(target_bone);
  glm::mat4 offsetMat =
      glm::translate(glm::mat4(1.0f), offset_position) *
      glm::mat4_cast(glm::quat(glm::radians(offset_rotation))) *
      glm::scale(glm::mat4(1.0f), offset_scale);
  glm::mat4 finalWorld = boneWorld * offsetMat;

  owner->SetWorldPosition(glm::vec3(finalWorld[3]));

  glm::mat3 rot(finalWorld);
  for (int i = 0; i < 3; ++i) {
    float len = glm::length(rot[i]);
    if (len > 1e-8f)
      rot[i] /= len;
  }
  owner->SetWorldRotation(glm::normalize(glm::quat_cast(rot)));
}