//
// Created by malan on 30.04.2026.
//

#include "Scene.hpp"
#include "JsonFileBase.hpp"
#include "Util/FileUtil.hpp"
#include "Util/LoggerUtil.hpp"
#include <cmath>
#include <memory>
#include <stdexcept>

namespace Engine {
Scene::Scene(std::string path) {
  std::string res;
  if (FileUtil::ReadFile(path, &res) != 0) {
    SPDLOG_LOGGER_CRITICAL(ENGINE_UTIL_LOGGER, "Failed to open scene at {}",
                           path);
    throw std::logic_error("Failed to open scene at " + path);
  }
  json js;
  try {
    js = json::parse(res);
  } catch (const std::exception &e) {
    SPDLOG_LOGGER_CRITICAL(ENGINE_UTIL_LOGGER,
                           "Failed to parse json scene at {} ({})", path,
                           e.what());
    throw std::logic_error("Failed to parse json scene at " + path + " " +
                           e.what());
  }
  FromJson(js);
}

json Scene::ToJson() {
  SceneJson sj;
  sj.objects = {};
  for (auto var : objects) {
    sj.objects.insert(sj.objects.end(), var->ToJson());
  }
  return sj;
}

void SceneObject::FromJson(JsonFileBase jsbase,
                           std::shared_ptr<SceneObject> parent) {
  if (jsbase.object_type != ObjectType::Object) {
    throw std::logic_error("Bad object type");
  }
  SceneObjectJson js = jsbase.data;
  this->Parent = parent;
  if (parent != nullptr)
    parent->Children.insert(parent->Children.begin(), this->shared_from_this());
  auto o = std::make_shared<Object>();
  auto ojsref = ((json)js.data);
  o->FromJson(ojsref);
  this->instance = o;
  for (auto var : js.children) {
    auto j = std::make_shared<SceneObject>();
    j->FromJson(var);
    this->Children.insert(this->Children.end(), j);
  }
}

void SceneObject::SetParent(std::shared_ptr<SceneObject> parent) {
  std::erase(this->Parent->Children, this->shared_from_this());
  this->Parent = parent;
  parent->Children.insert(parent->Children.end(), this->shared_from_this());
}

JsonFileBase SceneObject::ToJson() {
  auto ret = std::make_shared<SceneObjectJson>();
  try {
    ret->data = instance->ToJson();
  } catch (const std::exception &e) {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                        "Could not convert object to json: {}", e.what());
  }
  std::vector<JsonFileBase> child = {};
  for (auto scene_object_json : this->Children) {
    child.insert(child.end(), (scene_object_json->ToJson()));
  }
  ret->children = child;
  JsonFileBase retbase;
  retbase.object_type = ObjectType::Object;
  retbase.data = *ret;
  return retbase;
}

void Scene::FromJson(json &js) {
  SceneJson ojs;
  try {
    ojs = js;
  } catch (const std::exception &e) {
    SPDLOG_LOGGER_CRITICAL(ENGINE_UTIL_LOGGER,
                           "Failed to convert json to scene ({})", e.what());
    throw std::logic_error("Failed to convert json to scene " +
                           (std::string)e.what());
  }
  this->objects = {};
  for (auto var : ojs.objects) {
    try {
      auto j = std::make_shared<SceneObject>();
      j->FromJson(var);
      this->objects.insert(this->objects.end(), j);
    } catch (const std::exception &e) {
      SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                          "Failed to parse scene object: ({})", e.what());
      continue;
    }
  }
}

Scene::Scene() { this->objects = {}; }
void Scene::Instantiate(std::shared_ptr<Object> object,
                        std::shared_ptr<SceneObject> Parent) {
  auto so = std::make_shared<SceneObject>();
  so->Parent = Parent;
  if (Parent != nullptr) {
    Parent->Children.insert(Parent->Children.end(), so);
  } else {
    objects.insert(objects.end(), so);
  }
  so->instance = object;
  so->Children = {};
}
} // namespace Engine
