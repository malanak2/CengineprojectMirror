//
// Created by malan on 30.04.2026.
//
#pragma once
#include "Graphics/Components/CameraComponent.hpp"
#include "JsonFileBase.hpp"
#include <nlohmann/json.hpp>
#include <vector>

#include "Object.hpp"

namespace Engine {
class Scene;
class SceneObjectJson {
public:
  std::vector<JsonFileBase> children;
  JsonFileBase data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SceneObjectJson, children, data)
//!
//! @brief Wrapper for object - used in scenes
//!
class SceneObject : public std::enable_shared_from_this<SceneObject> {
public:
  //!
  //! @instance Ptr to object
  //!
  std::shared_ptr<Engine::Object> instance;
  //!
  //! @Parent Ptr ro parent sceneObject
  //!
  std::weak_ptr<SceneObject> Parent;
  //!
  //! @Children Ptr to children of object
  //!
  std::vector<std::shared_ptr<SceneObject>> Children;
  JsonFileBase ToJson();
  void SetParent(std::shared_ptr<SceneObject> parent);
  std::shared_ptr<Graphics::CameraComponent>
  FromJson(JsonFileBase jsbase, std::shared_ptr<Scene> scene,
           std::shared_ptr<SceneObject> self,
           std::shared_ptr<SceneObject> parent = nullptr);
  //!
  //! @brief Calls setup on itself and then its children
  //!
  //!
  void Setup();
  //!
  //! @brief Calls Update on itself and its children
  //!
  //!
  void Update();
};

class SceneJson {
public:
  std::vector<JsonFileBase> objects;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SceneJson, objects)

class Scene : public IJson, public std::enable_shared_from_this<Scene> {
public:
  static std::shared_ptr<Scene> Load(std::string path);
  std::string path;
  Scene();
  // TODO: Maybe change to be one root object that is automatically there? idk
  //!
  //! @objects Pointer to root objects
  //!
  std::vector<std::shared_ptr<SceneObject>> objects;

  //!
  //! @brief Instantiate an object
  //!
  //! @param[in] object Object to spawn in the scene
  //! @param[in] Parent Pointer to the sceneobject to be its parent. If not set
  //! spawn in the root
  //!
  void Instantiate(std::shared_ptr<Object> object,
                   std::shared_ptr<SceneObject> Parent = {});

  json ToJson() override;
  std::shared_ptr<Graphics::CameraComponent> camera = nullptr;
  void FromJson(json &js) override;
  void FromJson(json &js, std::shared_ptr<Scene> self);
  void Setup();
  void Update();
};
} // namespace Engine
