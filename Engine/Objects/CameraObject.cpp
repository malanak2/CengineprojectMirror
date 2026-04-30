#include "CameraObject.hpp"
#include "Graphics/Components/ComponentRenderable.hpp"
#include "Interfaces/IComponent.hpp"
#include "JsonFileBase.hpp"
#include "Util/FileUtil.hpp"
#include "Util/LoggerUtil.hpp"
#include <exception>
#include <memory>
#include <unordered_map>

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
  fromParams(name, comps, position, rotation);
}
void CameraObject::fromParams(std::string name,
                              std::vector<std::shared_ptr<IComponent>> comps,
                              std::vector<float> position,
                              std::vector<float> rotation) {
  _name = name;
  for (auto cmp : comps) {
    _components[cmp->GetType()] = cmp;
  }
  this->_position = position;
  this->_rotation = rotation;
}

CameraObject::CameraObject(std::string path) {
  // TODO:
  // Load data...
  std::string js;
  if (FileUtil::ReadFile(path, &js) != 0) {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Failed to open CameraObject at {}",
                        path);
    return;
  }
  json j;
  JsonFileBase jsbase;
  CameraObjectJson jsobj;
  try {
    j = json::parse(js);
    jsbase = j;
    if (jsbase.object_type != ObjectType::Object)
      throw std::exception();
  } catch (const std::exception &e) {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                        "Failed to parse CameraObject/Tried to load a "
                        "different obj type at {}",
                        path);
    return;
  }
  jsobj = jsbase.data;
  std::vector<std::shared_ptr<IComponent>> cmps;
  for (auto c : jsobj.components) {
    if (c.object_type != ObjectType::Component) {
      SPDLOG_LOGGER_ERROR(
          ENGINE_UTIL_LOGGER,
          "Object at {} contains a 'component' with the obj type {}, ingoring",
          path, (int)c.object_type);
      continue;
    }
    switch (c.type) {
    case renderable: {
      std::shared_ptr<Graphics::ComponentRenderable> r =
          std::make_shared<Graphics::ComponentRenderable>(c.data);
      cmps.insert(cmps.begin(), r);
      break;
    }
    default:
      SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Component not implemented in {}",
                          path);
      break;
    }
  }
  fromParams(jsobj.name, cmps, jsobj.position, jsobj.rotation);
}

CameraObject::~CameraObject() {}

json CameraObject::ToJson() {
  JsonFileBase ret;
  ret.object_type = ObjectType::Object;
  CameraObjectJson js;
  std::vector<ComponentJson> cmps = {};
  int i = 0;
  for (const auto& [key, val] : this->_components) {
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
