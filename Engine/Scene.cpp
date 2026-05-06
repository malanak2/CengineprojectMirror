//
// Created by malan on 30.04.2026.
//

#include "Scene.hpp"
#include "Graphics/Components/CameraComponent.hpp"
#include "Interfaces/IComponent.hpp"
#include "JsonFileBase.hpp"
#include "Util/FileUtil.hpp"
#include "Util/LoggerUtil.hpp"
#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>

namespace Engine {
std::shared_ptr<Scene> Scene::Load(std::string path) {
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
  auto scene = std::make_shared<Scene>();
  scene->path = path;
  scene->path.reserve(100);
  scene->FromJson(js, scene);
  return scene;
}

json Scene::ToJson() {
  SceneJson sj;
  sj.objects = {};
  for (auto var : objects) {
    sj.objects.insert(sj.objects.end(), var->ToJson());
  }
  return sj;
}

std::shared_ptr<Graphics::CameraComponent>
SceneObject::FromJson(JsonFileBase jsbase, std::shared_ptr<Scene> scene,
                      std::shared_ptr<SceneObject> self,
                      std::shared_ptr<SceneObject> parent) {
  std::shared_ptr<Graphics::CameraComponent> camcomp = nullptr;
  if (jsbase.object_type != ObjectType::Object) {
    throw std::logic_error("Bad object type");
  }
  SceneObjectJson js = jsbase.data;
  this->Parent = parent;
  auto o = std::make_shared<Object>(scene);
  auto ojsref = ((json)js.data);
  auto rr = o->FromJson(ojsref, nullptr);
  if (rr != nullptr) {
    camcomp = rr;
  }
  this->instance = o;
  for (auto var : js.children) {
    auto j = std::make_shared<SceneObject>();
    auto r = j->FromJson(var, scene, j, self);
    if (r != nullptr)
      camcomp = r;

    this->Children.insert(this->Children.end(), j);
  }
  return camcomp;
}

void SceneObject::SetParent(std::shared_ptr<SceneObject> parent) {
  auto oldParent = this->Parent.lock();
  if (oldParent) {
    std::erase(oldParent->Children, this->shared_from_this());
  }
  this->Parent = parent;
  if (parent) {
    parent->Children.insert(parent->Children.end(), this->shared_from_this());
  }
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
  FromJson(js, shared_from_this());
}

void Scene::FromJson(json &js, std::shared_ptr<Scene> self) {
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
      auto c = j->FromJson(var, self, j);
      if (this->camera == nullptr && c != nullptr) {
        this->camera = c;
      }
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
  object->scene = shared_from_this();
  so->instance = object;
  so->Children = {};
}
void Scene::Setup() {
  for (auto var : this->objects) {
    var->Setup();
  }
}
void SceneObject::Setup() {

  instance->Setup();
  for (auto var : this->Children) {
    var->Setup();
  }
}
void Scene::Update() {
  for (auto var : this->objects) {
    var->Update();
  }
}
void SceneObject::Update() {
  instance->Update();
  for (auto var : this->Children) {
    var->Update();
  }
}
} // namespace Engine
  //
