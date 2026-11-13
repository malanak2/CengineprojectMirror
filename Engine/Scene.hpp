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
using CameraList = std::vector<std::shared_ptr<Graphics::CameraComponent>>;
class SceneObjectJson {
public:
  std::vector<JsonFileBase> children;
  JsonFileBase data;
  bool isEnabled = true;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SceneObjectJson, children, data, isEnabled)
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
  
  
  //!
  //! @brief Whether the objet is enabled - its update mainly
  //!
  bool IsEnabled() const;

  //! 
  //! @brief Sets if enabled, then calls the proper method if changed
  //! @param enabled 
  void SetIsEnabled(bool enabled)
  {
    if (enabled == isEnabled) return;
    isEnabled = enabled;
    if (enabled)
    {
      OnEnable();
    } else
    {
      OnDisable();
    }
  }
  
  virtual JsonFileBase ToJson();
  SceneObject() {}
  SceneObject(std::shared_ptr<SceneObject> other) noexcept
      : instance(other ? other->instance : nullptr),
        Parent(other ? other->Parent : std::weak_ptr<SceneObject>()),
        scene(other ? other->scene : std::weak_ptr<Scene>()),
        Children(other ? other->Children
                       : std::vector<std::shared_ptr<SceneObject>>()) {}
  virtual ~SceneObject() = default;
  virtual void SetParent(const std::shared_ptr<SceneObject> &parent);
  virtual CameraList FromJson(const JsonFileBase &jsbase,
                              const std::shared_ptr<Scene> &owner,
                              const std::shared_ptr<SceneObject> &parent);
  static std::shared_ptr<SceneObject>
  CreateFromJson(const JsonFileBase &js, const std::shared_ptr<Scene> &owner,
                 const std::shared_ptr<SceneObject> &parent, CameraList &cameras);
  std::shared_ptr<Scene> GetRootScene();
  static std::shared_ptr<Scene> OwnerOf(const std::shared_ptr<SceneObject> &so);
  void SetOwner(const std::shared_ptr<Scene> &owner);
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
  //!
  //! @brief Called when object enabled
  //!
  virtual void OnEnable();
  //!
  //! @brief Called when object disabled
  //!
  virtual void OnDisable();

  std::shared_ptr<Graphics::CameraComponent>
  BuildInstance(const json &objectJson, const std::shared_ptr<Scene> &owner);
  CameraList LoadChildren(const std::vector<JsonFileBase> &children,
                          const std::shared_ptr<Scene> &owner);
protected: 
  //!
  //! @isEnabled Whether the objet is enabled - its update mainly
  //!
  bool isEnabled = true;
};
   //!
   //! @brief One object's changes relative to the prefab file
   //!
   struct PrefabOverride {
     std::vector<int> path;
     json patch;            
   };
   NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PrefabOverride, path, patch)

   //!
   //! @brief Prefab entry in a scene: the file to load + overrides on top of it
   //!
   class PrefabJson : public SceneObjectJson {
   public:
     std::string prefab_path;
     std::vector<PrefabOverride> overrides;
   };
   NLOHMANN_DEFINE_DERIVED_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
       PrefabJson, SceneObjectJson, prefab_path, overrides);

   class PrefabSceneObject : public SceneObject {
   public:
     std::string prefab_path;
     //!
     //! @brief Scene save: writes only the overrides (does NOT touch the prefab file)
     //!
     JsonFileBase ToJson() override;
     CameraList FromJson(const JsonFileBase &jsbase,
                         const std::shared_ptr<Scene> &owner,
                         const std::shared_ptr<SceneObject> &parent) override;
     bool SavePrefab();
     PrefabSceneObject(std::shared_ptr<SceneObject> so) : SceneObject(so) {
       if (this->instance) {
         this->prefab_path =
             "resources/prefabs/" + this->instance->_name + ".json";
       }
     }
     PrefabSceneObject() {}

   private:
     std::vector<PrefabOverride> overrides;
     //! Serialized state of each prefab object as it is in the prefab file
     std::map<std::vector<int>, json> baseJson;
     //! Live objects that came from the prefab file, with their prefab path
     std::vector<std::pair<std::weak_ptr<SceneObject>, std::vector<int>>> nodes;
     bool hasBase = false;
     CameraList LoadNode(const std::shared_ptr<SceneObject> &node,
                         const SceneObjectJson &js,
                         const std::vector<int> &path,
                         const std::shared_ptr<Scene> &owner);
     void CaptureTracking(const std::shared_ptr<SceneObject> &node,
                          const std::vector<int> &path);
     std::vector<PrefabOverride> ComputeOverrides();
   };
  
class SceneRefJson : public SceneObjectJson {
public:
  std::string scene_path;
};
NLOHMANN_DEFINE_DERIVED_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SceneRefJson, SceneObjectJson, scene_path);

class SceneJson : public SceneObjectJson {
public:
  std::string default_camera;
};
NLOHMANN_DEFINE_DERIVED_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SceneJson, SceneObjectJson, default_camera);

class Scene : public SceneObject {
public:
  static std::shared_ptr<Scene> Create();
  static std::shared_ptr<Scene> Load(std::string path);
  Scene() {}

  std::string path;
  bool loadFailed = false;
  bool Reload();
  std::shared_ptr<Scene> SelfScene() {
    return std::static_pointer_cast<Scene>(shared_from_this());
  }
  bool IsRootScene() const { return Parent.expired(); }

  std::shared_ptr<Scene> InstantiateScene(const std::string &path,
                                          const std::shared_ptr<SceneObject> &parent = {});
  void Instantiate(const std::shared_ptr<Object> &object,
                   const std::shared_ptr<SceneObject> &Parent = {},
                   bool asPrefab = false);
  void RemoveObject(const std::shared_ptr<SceneObject> &so);
  void RemoveObjectCascade(const std::shared_ptr<SceneObject> &so);

  JsonFileBase ToJson() override;
  CameraList FromJson(const JsonFileBase &jsbase,
                      const std::shared_ptr<Scene> &owner,
                      const std::shared_ptr<SceneObject> &parent) override;
  bool Save();
  std::shared_ptr<Object> FindObject(const std::string &name);
  void FinalizeCameras(const CameraList &found);

  std::shared_ptr<Graphics::CameraComponent> camera = nullptr; // root scene only
  std::string default_camera;                                  // root scene only
  CameraList cameras;                                          // root scene only
  void Unload();
  bool isInside(std::shared_ptr<SceneObject> o)
  {
    for (auto p = o; p; p = p->Parent.lock())
      if (p.get() == this)
        return true;
    return false;
  }

private:
  void Init();
  CameraList LoadFromFile();
  bool WouldCycle() const;
  void ApplyRootOverrides(const json &objectJson);
  JsonFileBase FileToJson();
  JsonFileBase ReferenceToJson();
  json rawReference;
};
} // namespace Engine