#include "Object.hpp"
#include "Engine.hpp"
#include "Graphics/Components/CameraComponent.hpp"
#include "Graphics/Components/ComponentRenderable.hpp"
#include "Interfaces/IComponent.hpp"
#include "JsonFileBase.hpp"
#include "ScriptSystem.hpp"
#include "Util/FileUtil.hpp"
#include "Util/LoggerUtil.hpp"
#include "spdlog/spdlog.h"
#include <glm/ext/matrix_transform.hpp>
#include <memory>
#include <utility>

namespace Engine {

Object::Object(std::string path, const std::shared_ptr<Scene> &scene) {
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
Object::Object() {
  this->_components = {};
  this->_localPosition = {0, 0, 0};
  this->_worldPosition = {0, 0, 0};
  this->_localRotation = glm::identity<glm::quat>();
  this->_worldRotation = glm::identity<glm::quat>();
  this->_name = "New Object";
  this->scale = {1, 1, 1};
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
    j.data = val_js->ToJson();
    j.name = val->GetName();
    cmps.insert(cmps.end(), static_cast<ComponentJson>(j));
  }
  js.components = cmps;
  js.position = {_localPosition[0], _localPosition[1], _localPosition[2]};
  js.rotation = {_localRotation.w, _localRotation.x, _localRotation.y,
                 _localRotation.z};
  js.name = _name;
  js.scale = std::vector<float>{scale.x, scale.y, scale.z};
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
  try {
    jsbase = js;
    if (jsbase.object_type != ObjectType::Object)
      throw std::logic_error("Not of type object");
    jsobj = jsbase.data;
  } catch (const std::exception &e) {
    SPDLOG_LOGGER_ERROR(
        ENGINE_UTIL_LOGGER,
        "Failed to parse Object/Tried to load a different obj type at {} ({})",
        path, e.what());
    throw; // was `throw e;`, which sliced the exception
  }

  std::shared_ptr<Object> self;
  try {
    self = this->shared_from_this();
  } catch (const std::bad_weak_ptr &) {
    throw std::logic_error(
        "Object::FromJson needs the object to be owned by a std::shared_ptr");
  }

  std::shared_ptr<Graphics::CameraComponent> cc = nullptr;
  std::vector<std::shared_ptr<IComponent>> cmps;
  cmps.reserve(jsobj.components.size());
  for (auto &c : jsobj.components) {
    auto script =
        Main::ScriptSystem::instance->GetScript(c.name); // was fetched twice
    if (!script) {
      SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER,
                         "Skipping unknown component {} in {}", c.name, path);
      continue;
    }
    script->FromJson(c.data);
    script->SetObject(self);
    cmps.insert(cmps.begin(), script);
    if (auto cam = std::dynamic_pointer_cast<Graphics::CameraComponent>(script))
      cc = cam; // by type, not by name
  }

  glm::vec3 scale(1.0f);
  if (jsobj.scale.size() >= 3) {
    scale = glm::vec3(jsobj.scale[0], jsobj.scale[1], jsobj.scale[2]);
  } else {
    SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER,
                       "Object {} in {} has a bad scale (size {}), using 1",
                       jsobj.name, path, jsobj.scale.size());
  }
  fromParams(jsobj.name, cmps, jsobj.position, jsobj.rotation, scale);
  return cc;
}

void Object::fromParams(std::string name,
                        const std::vector<std::shared_ptr<IComponent>> &comps,
                        std::vector<float> position,
                        std::vector<float> rotation, glm::vec3 scale) {
  _name = std::move(name);
  _name.reserve(50);
  for (const auto &cmp : comps) {
    _components[std::string(cmp->GetName())] = cmp;
  }
  if (position.size() != 3) {
    SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER, "Bad position passed to Object : {}",
                       position.size());
  } else {
    this->_localPosition = glm::vec3(position[0], position[1], position[2]);
  }

  this->_worldPosition = _localPosition;
  if (rotation.size() == 4) {
    this->_localRotation =
        glm::quat(rotation[0], rotation[1], rotation[2], rotation[3]);

  } else if (rotation.size() == 3) {
    this->_localRotation = glm::quat(
        glm::radians(glm::vec3(rotation[0], rotation[1], rotation[2])));
  } else {
    SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER, "Bad rotation passed to Object : {}",
                       rotation.size());
    this->_localRotation = glm::identity<glm::quat>();
  }

  this->_worldRotation = _localRotation;
  this->scale = scale;
}

Object::Object(const std::shared_ptr<Scene> &scene) : Object() {
  this->scene = scene;
}
void Object::EarlySetup() {
  for (auto &[key, comp] : this->_components) {
    comp->EarlySetup();
  }
}
void Object::Setup() {
  for (auto &[key, comp] : this->_components) {
    if (comp->FollowsPausedExecution() && !Engine::Engine::isSimulationEnabled)
      continue;
    comp->Setup();
  }
}
void Object::Update() {
  for (auto &[key, comp] : this->_components) {
    if (comp->FollowsPausedExecution() && !Engine::Engine::isSimulationEnabled)
      continue;
    comp->Update();
  }
}
void Object::End() {
  for (auto &[id, comp] : _components) {
    if (comp->FollowsPausedExecution() && !Engine::Engine::isSimulationEnabled)
      continue;
    comp->End();
  }
}
void Object::LateEnd() {
  for (auto &[id, comp] : _components) {
    comp->LateEnd();
  }
}
void Object::GetParentWorld(glm::vec3 &pos, glm::quat &rot) const {
  pos = {0, 0, 0};
  rot = glm::identity<glm::quat>();
  if (auto so = sceneobject.lock())
    if (auto p = so->Parent.lock())
      if (p->instance) {
        pos = p->instance->GetPosition();
        rot = p->instance->GetRotation();
      }
}
void Object::SetWorldPosition(glm::vec3 w) {
  glm::vec3 pp;
  glm::quat pr;
  GetParentWorld(pp, pr);
  SetPosition(glm::inverse(pr) * (w - pp));
}
void Object::SetWorldRotation(glm::quat w) {
  glm::vec3 pp;
  glm::quat pr;
  GetParentWorld(pp, pr);
  SetRotation(glm::normalize(glm::inverse(pr) * w));
}
void Object::UpdateWorldTransform() {
  // Compute world position: parent world pos + local position
  glm::vec3 parentWorldPos = {0, 0, 0};
  if (auto so = sceneobject.lock()) {
    if (auto parent = so->Parent.lock()) {
      if (parent->instance) {
        parentWorldPos = parent->instance->GetPosition();
      }
    }
  }
  glm::quat parentWorldRot = glm::identity<glm::quat>();
  if (auto so = sceneobject.lock()) {
    if (auto parent = so->Parent.lock()) {
      if (parent->instance) {
        parentWorldRot = parent->instance->GetRotation();
      }
    }
  }

  _worldPosition = parentWorldPos + parentWorldRot * _localPosition;
  _worldRotation = glm::normalize(parentWorldRot * _localRotation);
  // Propagate to children
  if (auto so = sceneobject.lock()) {
    for (auto &child : so->Children) {
      if (child && child->instance) {
        child->instance->UpdateWorldTransform();
      }
    }
  }
}

} // namespace Engine
