//
// Created by malan on 30.04.2026.
//

#include "Scene.hpp"
#include "Util/FileUtil.hpp"
#include "Util/LoggerUtil.hpp"
#include <memory>

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
}

// TODO:
json Scene::ToJson() {
  SceneJson sj;
  sj.objects = {};
  for (auto var : objects) {
    sj.objects.insert(sj.objects.end(), *(var->ToJson()));
  }
  return {};
}

void SceneObject::FromJson(SceneObjectJson js,
                           std::shared_ptr<SceneObject> parent) {
  this->Parent = parent;
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

std::shared_ptr<SceneObjectJson> SceneObject::ToJson() {
  auto ret = std::make_shared<SceneObjectJson>();
  ret->data = instance->ToJson();
  // TODO: Finish
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
                          "Failed to parse scene object: {}, skipping",
                          e.what());
      continue;
    }
  }
}

Scene::Scene() { this->objects = {}; }
void Scene::Instantiate(std::shared_ptr<Object> object,
                        std::shared_ptr<SceneObject> Parent) {
  auto so = std::make_shared<SceneObject>();
  so->Parent = Parent;
  so->instance = object;
  so->Children = {};
  objects.insert(objects.end(), so);
}
} // namespace Engine
