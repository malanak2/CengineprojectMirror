#include "CameraComponent.hpp"
#include "Engine.hpp"
#include "Interfaces/IComponent.hpp"
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include <memory>
namespace Engine {
namespace Graphics {

// TODO:
using json = nlohmann::json;
void CameraComponent::Setup() {
  Update();
}

void CameraComponent::Update() {
  auto obj = object.lock();
  if (!obj) return;
  glm::vec3 front;
  front.x = glm::cos(glm::radians(obj->_rotation.y)) *
            glm::cos(glm::radians(obj->_rotation.x));
  front.y = glm::sin(glm::radians(obj->_rotation.x));
  front.z = glm::sin(glm::radians(obj->_rotation.y)) *
            glm::cos(glm::radians(obj->_rotation.x));

  glm::vec3 frontnormal = glm::normalize(front);
  projmat = glm::perspective(
      glm::radians(45.0f),
      (float)Engine::Main::width / (float)Engine::Main::height, 0.1f, 100.0f);
  viewmat = glm::lookAt(obj->_position, obj->_position + frontnormal,
                        glm::vec3(0, 1, 0));
}

void CameraComponent::FixedUpdate() {}

void CameraComponent::Save() {}

void CameraComponent::Load() {}

CameraComponent::CameraComponent(std::shared_ptr<Object> object) {
  this->object = object;
  // fromParams(name, comps, position, rotation);
}

/*void CameraComponent::fromParams(std::string name,
                              std::vector<std::shared_ptr<IComponent> > comps,
                              std::vector<float> position,
                              std::vector<float> rotation) {
    _name = name;
    for (auto cmp: comps) {
        _components[cmp->GetType()] = cmp;
    }
    this->_position = position;
    this->_rotation = rotation;
}*/
/*
CameraComponent::CameraComponent*/

CameraComponent::~CameraComponent() {}
/*
json CameraComponent::ToJson() {
    JsonFileBase ret;
    ret.object_type = ObjectType::Object;
    CameraComponentJson js;
    std::vector<ComponentJson> cmps = {};
    int i = 0;
    for (const auto &[key, val]: this->_components) {
        ComponentJson j;
        j.object_type = ObjectType::Component;
        j.data = val->ToJson();
        j.type = val->GetType();
        cmps.insert(cmps.end(), static_cast<ComponentJson>(j));
    }
    js.components = cmps;
    js.position = _position;
    js.rotation = _rotation;
    js.name = _name;
    ret.data = js;
    json r = ret;
    return r;
}
*/

ENGINE_COMPONENT_TYPE CameraComponent::GetType() {
  return ENGINE_COMPONENT_TYPE::camera;
}

// TODO:
void CameraComponent::FromJson(json &js) {}

json CameraComponent::ToJson() { return {}; }
CameraComponent::CameraComponent(json &js, std::shared_ptr<Object> object) {
  this->object = object;
  FromJson(js);
} //
glm::mat4 CameraComponent::GetProjMatrix() { return projmat; }
glm::mat4 CameraComponent::GetViewMatrix() { return viewmat; }
} // namespace Graphics
} // namespace Engine
