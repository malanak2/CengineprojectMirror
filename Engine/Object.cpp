#include "Object.hpp"
#include "Graphics/Components/CameraComponent.hpp"
#include "Interfaces/IComponent.hpp"
#include "JsonFileBase.hpp"
#include "Util/FileUtil.hpp"
#include "Util/LoggerUtil.hpp"
#include "spdlog/spdlog.h"
#include <memory>

namespace Engine {

Object::Object(std::string path) {
  // TODO:
  // Load data...
  std::string js;
  if (FileUtil::ReadFile(path, &js) != 0) {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Failed to open Object at {}",
                        path);
    return;
  }
  json j;

  try {
    j = json::parse(js);
  } catch (const std::exception &e) {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Invalid json at {}", path);
    return;
  }
  FromJson(j);
}

json Object::ToJson() {
  JsonFileBase ret;
  ret.object_type = ObjectType::Object;
  ObjectJson js;
  std::vector<ComponentJson> cmps = {};
  int i = 0;
  for (const auto &[key, val] : this->_components) {
    ComponentJson j;
    auto val_js = std::static_pointer_cast<IJson>(val);
    j.object_type = ObjectType::Component;
    j.data = val_js->ToJson();
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

void Object::FromJson(json &js) { FromJson(js, "%internal%"); }

void Object::FromJson(json &js, std::string path) {
  JsonFileBase jsbase;
  ObjectJson jsobj;
  try {
    jsbase = js;
    if (jsbase.object_type != ObjectType::Object)
      throw std::exception();
  } catch (const std::exception &e) {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                        "Failed to parse Object/Tried to load a "
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
    case camera: {
      std::shared_ptr<Engine::Graphics::CameraComponent> r =
          std::make_shared<Engine::Graphics::CameraComponent>(c.data);
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

void Object::fromParams(std::string name,
                        std::vector<std::shared_ptr<IComponent>> comps,
                        std::vector<float> position,
                        std::vector<float> rotation) {
  _name = name;
  for (auto cmp : comps) {
    _components[cmp->GetType()] = cmp;
  }
  if (position.size() != 3) {
    SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER, "Bad position passed to Object : {}",
                       position.size());
  } else {
    this->_position = position;
  }
  if (rotation.size() != 4) {
    SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER, "Bad rotation passed to Object : {}",
                       position.size());
  } else {
    this->_rotation = rotation;
  }
}
} // namespace Engine
