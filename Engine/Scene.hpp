//
// Created by malan on 30.04.2026.
//
#pragma once
#include "JsonFileBase.hpp"
#include <nlohmann/json.hpp>
#include <vector>

#include "Object.hpp"

namespace Engine {
// nullptr Parent means its in root - multiple objects can be that
//
class SceneObjectJson {
public:
  std::vector<JsonFileBase> children;
  JsonFileBase data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SceneObjectJson, children, data)
class SceneObject : public std::enable_shared_from_this<SceneObject> {
public:
  std::shared_ptr<Engine::Object> instance;
  std::shared_ptr<SceneObject> Parent;
  std::vector<std::shared_ptr<SceneObject>> Children;
  JsonFileBase ToJson();
  void SetParent(std::shared_ptr<SceneObject> parent);
  void FromJson(JsonFileBase jsbase,
                std::shared_ptr<SceneObject> parent = nullptr);
};

class SceneJson {
public:
  std::vector<JsonFileBase> objects;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SceneJson, objects)

class Scene : public IJson {
public:
  Scene(std::string path);
  std::string path;
  Scene();
  std::vector<std::shared_ptr<SceneObject>> objects;
  void Instantiate(std::shared_ptr<Object> object,
                   std::shared_ptr<SceneObject> Parent = {});

  json ToJson() override;

  void FromJson(json &js) override;
};
} // namespace Engine
