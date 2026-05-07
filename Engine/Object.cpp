#include "Object.hpp"
#include "Graphics/Components/CameraComponent.hpp"
#include "Graphics/Components/ComponentRenderable.hpp"
#include "Interfaces/IComponent.hpp"
#include "JsonFileBase.hpp"
#include "Util/FileUtil.hpp"
#include "Util/LoggerUtil.hpp"
#include "spdlog/spdlog.h"
#include <memory>

namespace Engine {

Object::Object(std::string path, std::shared_ptr<Scene> scene) {
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
  this->scene = scene;
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
  js.position = {_position[0], _position[1], _position[2]};
  js.rotation = {_rotation[0], _rotation[1], _rotation[2]};
  js.name = _name;
  ret.data = js;
  json r = ret;
  return r;
}
void Object::FromJson(json &js) { FromJson(js, nullptr); }

std::shared_ptr<Graphics::CameraComponent> Object::FromJson(json &js, void *) {
  return FromJson(js, "%internal%");
}

std::shared_ptr<Graphics::CameraComponent> Object::FromJson(json &js,
                                                            std::string path) {
  JsonFileBase jsbase;
  ObjectJson jsobj;

  std::shared_ptr<Graphics::CameraComponent> cc = nullptr;
  try {
    jsbase = js;
    if (jsbase.object_type != ObjectType::Object)
      throw std::logic_error("Not of type objet");
  } catch (const std::exception &e) {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                        "Failed to parse Object/Tried to load a "
                        "different obj type at {}",
                        path);
    throw e;
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
          Graphics::ComponentRenderable::Create(c.data,
                                                this->shared_from_this());
      cmps.insert(cmps.begin(), r);
      break;
    }
    case camera: {
      std::shared_ptr<Engine::Graphics::CameraComponent> r =
          std::make_shared<Engine::Graphics::CameraComponent>(
              c.data, shared_from_this());
      cmps.insert(cmps.begin(), r);
      cc = r;
      break;
    }
    default:
      SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Component not implemented in {}",
                          path);
      break;
    }
  }
  fromParams(jsobj.name, cmps, jsobj.position, jsobj.rotation);
  return cc;
}

void Object::fromParams(std::string name,
                        std::vector<std::shared_ptr<IComponent>> comps,
                        std::vector<float> position,
                        std::vector<float> rotation) {
  _name = name;
  _name.reserve(50);
  for (auto cmp : comps) {
    _components[cmp->GetType()] = cmp;
  }
  if (position.size() != 3) {
    SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER, "Bad position passed to Object : {}",
                       position.size());
  } else {
    this->_position = glm::vec3(position[0], position[1], position[2]);
  }
  if (rotation.size() != 3) {
    SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER, "Bad rotation passed to Object : {}",
                       rotation.size());
  } else {
    this->_rotation = glm::vec3(rotation[0], rotation[1], rotation[2]);
  }
}
Object::Object(std::shared_ptr<Scene> scene) {
  this->scene = scene;
  this->_components = {};
  this->_position = {};
  this->_rotation = {};
}
void Object::Setup() {
  for (auto &[key, comp] : this->_components) {
    comp->Setup();
  }
}
void Object::Update() {
  for (auto &[key, comp] : this->_components) {
    comp->Update();
  }
}
} // namespace Engine
