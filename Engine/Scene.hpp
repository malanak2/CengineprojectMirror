//
// Created by malan on 30.04.2026.
//
#pragma once
#include <memory>
#include <nlohmann/detail/macro_scope.hpp>
#include <vector>

#include "Object.hpp"

namespace Engine {
// nullptr Parent means its in root - multiple objects can be that
//
class SceneObjectJson {
public:
  std::vector<SceneObjectJson> children;
  JsonFileBase data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SceneObjectJson, children, data)
class SceneObject : public std::enable_shared_from_this<SceneObject> {
public:
  std::shared_ptr<Engine::Object> instance;
  std::shared_ptr<SceneObject> Parent;
  std::vector<std::shared_ptr<SceneObject>> Children;
  std::shared_ptr<SceneObjectJson> ToJson();
  void SetParent(std::shared_ptr<SceneObject> parent);
  void FromJson(SceneObjectJson js,
                std::shared_ptr<SceneObject> parent = nullptr);
};

class SceneJson {
public:
  std::vector<SceneObjectJson> objects;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SceneJson, objects)

class Scene : public IJson {
public:
  Scene(std::string path);
  Scene();
  std::vector<std::shared_ptr<SceneObject>> objects;
  void Instantiate(std::shared_ptr<Object> object,
                   std::shared_ptr<SceneObject> Parent = {});

  json ToJson() override;

  void FromJson(json &js) override;
};
} // namespace Engine
