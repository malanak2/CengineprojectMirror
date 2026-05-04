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
class SceneObject {
public:
  std::shared_ptr<Engine::Object> instance;
  std::shared_ptr<SceneObject> Parent;
  std::vector<std::shared_ptr<SceneObject>> Children;
};
class SceneObjectJson {
public:
  std::vector<SceneObjectJson> children;
  std::string name;
  ObjectJson data;
  std::shared_ptr<SceneObject>
  Parse(std::shared_ptr<SceneObject> parent = nullptr);
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SceneObjectJson, children, name, data)
class SceneJson {
public:
  std::vector<SceneObjectJson> objects;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SceneJson, objects)

class Scene : public IJson {
public:
  Scene(std::string path);
  std::vector<std::shared_ptr<SceneObject>> objects;

  json ToJson() override;

  void FromJson(json &js) override;
};
} // namespace Engine
