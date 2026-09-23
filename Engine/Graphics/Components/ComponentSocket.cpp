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

  owner->_position = glm::vec3(finalWorld[3]);
  owner->_rotation = glm::quat_cast(finalWorld);
}

json Engine::Graphics::ComponentSocket::ToJson() {
  ComponentSocketJson data;
  data.target_object = target_object;
  data.target_bone = target_bone;
  data.position = {offset_position.x, offset_position.y, offset_position.z};
  data.rotation = {offset_rotation.x, offset_rotation.y, offset_rotation.z};
  data.scale = {offset_scale.x, offset_scale.y, offset_scale.z};
  json j;
  j["data"] = data;
  return j;
}

void Engine::Graphics::ComponentSocket::FromJson(json &js) {
  if (js.is_null())
    return;
  json data = js.contains("data") ? js["data"] : js;
  if (data.is_null())
    return;
  ComponentSocketJson sj = data.get<ComponentSocketJson>();
  target_object = sj.target_object;
  target_bone = sj.target_bone;
  if (sj.position.size() >= 3)
    offset_position =
        glm::vec3(sj.position[0], sj.position[1], sj.position[2]);
  if (sj.rotation.size() >= 3)
    offset_rotation =
        glm::vec3(sj.rotation[0], sj.rotation[1], sj.rotation[2]);
  if (sj.scale.size() >= 3)
    offset_scale = glm::vec3(sj.scale[0], sj.scale[1], sj.scale[2]);
}
