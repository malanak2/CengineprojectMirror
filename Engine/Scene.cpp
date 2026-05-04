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
json Scene::ToJson() { return {}; }

std::shared_ptr<SceneObject>
SceneObjectJson::Parse(std::shared_ptr<SceneObject> parent) {
  std::shared_ptr<SceneObject> ret = std::make_shared<SceneObject>();
  ret->Parent = parent;
  auto o = std::make_shared<Object>();
  auto ojsref = ((json)this->data);
  o->FromJson(ojsref);
  ret->instance = o;
  for (auto var : this->children) {
    ret->Children.insert(ret->Children.end(), var.Parse());
  }
  return ret;
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
      this->objects.insert(this->objects.end(), var.Parse());
    } catch (const std::exception &e) {
      SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                          "Failed to parse scene object: {}, skipping",
                          e.what());
      continue;
    }
  }
}
} // namespace Engine
