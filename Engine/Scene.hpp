//
// Created by malan on 30.04.2026.
//
#pragma once
#include "Graphics/Components/CameraComponent.hpp"
#include "JsonFileBase.hpp"
#include <nlohmann/detail/macro_scope.hpp>
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
  //! @scene Scene ptr
  //!
  std::weak_ptr<Scene> scene;
  //!
  //! @Children Ptr to children of object
  //!
  std::vector<std::shared_ptr<SceneObject>> Children;
  virtual JsonFileBase ToJson();
  virtual void SetParent(const std::shared_ptr<SceneObject> &parent);
  virtual std::vector<std::shared_ptr<Graphics::CameraComponent>>
  FromJson(const JsonFileBase &jsbase, const std::shared_ptr<Scene> &scene,
           const std::shared_ptr<SceneObject> &self,
           const std::shared_ptr<SceneObject> &parent);
  //!
  //! @brief Calls early setup on itself and then its children
  //!
  //!
  virtual void EarlySetup();

  //!
  //! @brief Calls setup on itself and then its children
  //!
  //!
  virtual void Setup();
  //!
  //! @brief Calls Update on itself and its children
  //!
  //!
  virtual void Update();
  //!
  //! @brief Calls end on itself and its children
  //!
  //!
  virtual void End();
  //!
  //! @brief Calls late end on itself and its children
  //!
  //!
  virtual void LateEnd();
};

// TODO: Save prefab file

//!
//! @brief Prefab - in the json, the SceneObjectJson are overrides
//!
class PrefabJson : public SceneObjectJson {
public:
  std::string prefab_path;
};
NLOHMANN_DEFINE_DERIVED_TYPE_NON_INTRUSIVE(PrefabJson, SceneObjectJson,
                                           prefab_path);

class PrefabSceneObject : public SceneObject {
public:
  std::string prefab_path;
  JsonFileBase ToJson() override;
  std::vector<std::shared_ptr<Graphics::CameraComponent>>
  FromJson(const JsonFileBase &jsbase, const std::shared_ptr<Scene> &scene,
           const std::shared_ptr<SceneObject> &self,
           const std::shared_ptr<SceneObject> &parent) override;
};

class SceneJson {
public:
  std::vector<JsonFileBase> objects;

  std::string default_camera;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SceneJson, objects, default_camera)

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
  //! @param[in] asPrefab Is prefab?
  //!
  void Instantiate(const std::shared_ptr<Object> &object,
                   const std::shared_ptr<SceneObject> &Parent = {},
                   bool asPrefab = false);
  void RemoveObject(const std::shared_ptr<SceneObject> &so);
  void RemoveObjectCascade(const std::shared_ptr<SceneObject> &so);
  json ToJson() override;
  void FromJson(json &js) override;
  void FromJson(json &js, std::shared_ptr<Scene> self);
  std::shared_ptr<Object> FindObject(const std::string &name);
  void Setup();
  void Update();
  std::shared_ptr<Graphics::CameraComponent> camera = nullptr;
  std::string default_camera;
  std::vector<std::shared_ptr<Graphics::CameraComponent>> cameras;
};
} // namespace Engine
