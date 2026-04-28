#include "Object.hpp"

using namespace Graphics;

Object::Object(std::string name, std::vector<std::shared_ptr<IComponent>> comps,
               std::vector<float> position) {
  _name = name;
  this->_components.insert(this->_components.end(), comps.begin(), comps.end());
  this->_position = position;
}

Object::~Object() {}

void Object::Setup() {
  for (auto &comp : _components) {
    comp->Setup();
  }
}

void Object::Update() {
  for (auto &comp : _components) {
    comp->Update();
  }
}

void Object::FixedUpdate() {
  for (auto &comp : _components) {
    comp->FixedUpdate();
  }
}

void Object::Save() {
  ObjectJson js;
  for (auto &comp : _components) {
    js.components.insert(js.components.end(), comp->ToJson());
  }
}

void Object::Load() {
  for (auto &comp : _components) {
    comp->Load();
  }
}

Object::Object(std::string path) {}
