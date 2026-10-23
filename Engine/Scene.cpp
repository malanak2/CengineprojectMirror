//
// Created by malan on 30.04.2026.
//

#include "Scene.hpp"
#include "Engine.hpp"
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
  sj.default_camera = default_camera;
  return sj;
}

std::vector<std::shared_ptr<Graphics::CameraComponent>>
SceneObject::FromJson(JsonFileBase jsbase, std::shared_ptr<Scene> scene,
                      std::shared_ptr<SceneObject> self,
                      std::shared_ptr<SceneObject> parent) {
  std::vector<std::shared_ptr<Graphics::CameraComponent>> camcomps = {};
  if (jsbase.object_type != ObjectType::Object) {
    throw std::logic_error("Bad object type");
  }
  SceneObjectJson js = jsbase.data;
  this->Parent = parent;
  auto o = std::make_shared<Object>(scene);
  auto ojsref = ((json)js.data);
  auto rr = o->FromJson(ojsref, nullptr);
  o->sceneobject = shared_from_this();
  if (rr != nullptr) {
    camcomps.push_back(rr);
  }

  this->instance = o;
  o->_UpdateWorldTransform();
  this->scene = std::weak_ptr<Scene>(scene);
  for (auto var : js.children) {
    auto j = std::make_shared<SceneObject>();
    auto r = j->FromJson(var, scene, j, self);
    camcomps.insert(camcomps.end(), r.begin(), r.end());
    this->Children.insert(this->Children.end(), j);
  }
  return camcomps;
}
void SceneObject::EarlySetup() {
  instance->EarlySetup();
  for (auto var : this->Children) {
    var->EarlySetup();
  }
}

void SceneObject::SetParent(std::shared_ptr<SceneObject> parent) {
  auto oldParent = this->Parent.lock();
  if (oldParent) {
    std::erase(oldParent->Children, this->shared_from_this());
  } else {
    std::erase(scene.lock()->objects, this->shared_from_this());
  }
  this->Parent = parent;
  if (parent) {
    parent->Children.insert(parent->Children.end(), this->shared_from_this());
  }
  if (instance)
    instance->_UpdateWorldTransform();
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

void Scene::FromJson(json &js) { FromJson(js, shared_from_this()); }

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
  this->cameras = {};
  this->default_camera = ojs.default_camera;
  for (auto var : ojs.objects) {
    try {
      auto j = std::make_shared<SceneObject>();
      auto c = j->FromJson(var, self, j);
      cameras.insert(cameras.end(), c.begin(), c.end());
      this->objects.insert(this->objects.end(), j);
    } catch (const std::exception &e) {
      SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                          "Failed to parse scene object: ({})", e.what());
      continue;
    }
  }
  if (cameras.size() == 0) {
    SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER,
                       "Scene {} has no cameras, creating default camera",
                       this->path);
    auto camobj = std::make_shared<SceneObject>();
    camobj->instance = std::make_shared<Object>(self);
    camobj->instance->_name = "Default Camera";
    camobj->instance->AddComponent<Graphics::CameraComponent>();
    auto cam = camobj->instance->GetComponent<Graphics::CameraComponent>();
    camobj->scene = self;
    cam->near = 0.1f;
    cam->far = 1000.0f;
    this->objects.insert(this->objects.end(), camobj);
    this->cameras.insert(this->cameras.end(), cam);
    return;
  }
  for (auto cam : cameras) {
    if (cam->object.lock()->_name == ojs.default_camera) {
      this->camera = cam;
      return;
    }
  }
  SPDLOG_LOGGER_ERROR(
      ENGINE_UTIL_LOGGER,
      "Scene {} is missing its default camera, using first camera", this->path);
  this->camera = cameras[0];
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
  object->sceneobject = so;
  so->instance = object;
  so->EarlySetup();
  so->Setup();
}
void Scene::RemoveObject(std::shared_ptr<SceneObject> so) {
  so->End();
  so->LateEnd();
  auto pos = std::find(objects.begin(), objects.end(), so);
  if (pos != objects.end()) {
    objects.erase(pos);
    auto chls = std::vector<std::shared_ptr<SceneObject>>{};
    for (auto ch : so->Children) {
      objects.insert(objects.end(), ch);
      chls.push_back(ch);
    }
    for (auto ch : chls) {
      ch->SetParent(nullptr);
    }
    return;
  }
  auto parent = so->Parent.lock();
  auto loc = std::find(parent->Children.begin(), parent->Children.end(), so);
  if (loc != parent->Children.end()) {
    parent->Children.erase(loc);
  }
  auto chls = std::vector<std::shared_ptr<SceneObject>>{};
  for (auto child : so->Children) {
    child->SetParent(parent);
  }
  for (auto ch : chls) {

    ch->SetParent(parent);
  }
}

void Scene::RemoveObjectCascade(std::shared_ptr<SceneObject> so) {
  auto ch = std::vector<std::shared_ptr<SceneObject>>(so->Children);
  for (auto child : ch) {
    RemoveObjectCascade(child);
  }
  so->End();
  so->LateEnd();
  if (auto p = so->Parent.lock()) {
    auto pos = std::find(p->Children.begin(), p->Children.end(), so);
    if (pos == p->Children.end()) {
      SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "What?");
    } else {
      p->Children.erase(pos);
    }
  }

  auto pos = std::find(objects.begin(), objects.end(), so);
  if (pos == objects.end()) {
    return;
  }
  objects.erase(pos);
}

void Scene::Setup() {
  for (auto var : this->objects) {
    var->EarlySetup();
  }
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
void SceneObject::End() { instance->End(); }
void SceneObject::LateEnd() {
  instance->LateEnd();
  for (auto var : this->Children) {
    var->LateEnd();
  }
}

static std::shared_ptr<Object>
FindObjectRecursive(const std::shared_ptr<SceneObject> &so,
                    const std::string &name) {
  if (so && so->instance && so->instance->_name == name)
    return so->instance;
  if (so) {
    for (const auto &child : so->Children) {
      auto res = FindObjectRecursive(child, name);
      if (res)
        return res;
    }
  }
  return nullptr;
}

std::shared_ptr<Object> Scene::FindObject(const std::string &name) {
  for (const auto &rootSo : objects) {
    auto res = FindObjectRecursive(rootSo, name);
    if (res)
      return res;
  }
  return nullptr;
}
} // namespace Engine
