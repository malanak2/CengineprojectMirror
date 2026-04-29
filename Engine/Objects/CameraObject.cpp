#include "CameraObject.hpp"

// TODO:

void CameraObject::Setup() {}

void CameraObject::Update() {}

void CameraObject::FixedUpdate() {}

void CameraObject::Save() {}

void CameraObject::Load() {}
CameraObject::CameraObject(std::string name,
                           std::vector<std::shared_ptr<IComponent>> comps,
                           std::vector<float> position,
                           std::vector<float> rotation) {
  _name = name;
  this->_components.insert(this->_components.end(), comps.begin(), comps.end());
  this->_position = position;
  this->_rotation = rotation;
}

CameraObject::CameraObject(std::string path) {}

CameraObject::~CameraObject() {}

json CameraObject::ToJson() {
  json ret;
  return ret;
}
