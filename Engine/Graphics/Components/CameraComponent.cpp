#include "CameraComponent.hpp"
#include "Interfaces/IComponent.hpp"
namespace Engine {
namespace Graphics {

// TODO:
using json = nlohmann::json;
void CameraComponent::Setup() {}

void CameraComponent::Update() {}

void CameraComponent::FixedUpdate() {}

void CameraComponent::Save() {}

void CameraComponent::Load() {}

CameraComponent::CameraComponent() {
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

void CameraComponent::FromJson(json &js) {}

json CameraComponent::ToJson() { return {}; }
CameraComponent::CameraComponent(json &js) { FromJson(js); } //
} // namespace Graphics
} // namespace Engine
