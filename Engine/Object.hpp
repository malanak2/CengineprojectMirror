#pragma once

#include "Graphics/Components/CameraComponent.hpp"
#include "Graphics/Components/ComponentRenderable.hpp"
#include "Interfaces/IComponent.hpp"
#include "ScriptSystem.hpp"
#include "Util/LoggerUtil.hpp"
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <string>
#include <tracy/Tracy.hpp>
#include <vector>
using json = nlohmann::json;

namespace Engine {
class Scene;
class SceneObject;
//!
//! struct ObjectJson - Json struct for saving objects
//!
struct ObjectJson {
  std::vector<float> position;
  std::vector<float> rotation;
  std::vector<float> scale;
  std::vector<ComponentJson> components;
  std::string name;
  bool inherit_parent_rotation = false;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    ObjectJson, position, rotation, scale, components, name,
    inherit_parent_rotation)
//!
//! @brief Scene object. Basic building block
//!
class Object : public IJson, public std::enable_shared_from_this<Object> {
public:
  //!
  //! @_components All components on the object. Since the object can only have
  //! one of each.
  //!
  std::map<std::string, std::shared_ptr<IComponent>> _components = {};
  //!
  //! @brief Get the world-space position of the object (local + parent offset)
  //!
  glm::vec3 GetPosition() const { return _worldPosition; }
  //!
  //! @brief Get the local position of the object (relative to parent)
  //!
  glm::vec3 GetLocalPosition() const { return _localPosition; }

  //!
  //! @brief Set the local position of the object. Propagates to children.
  //!
  void SetPosition(glm::vec3 pos) {
    _localPosition = pos;
    UpdateWorldTransform();
  }

  //!
  //! @brief Add a delta to the local position. Propagates to children.
  //!
  void AddPosition(glm::vec3 delta) { SetPosition(_localPosition + delta); }
  //!
  //! @brief Get the world-space rotation of the object (parent rot * local rot)
  //!
  glm::quat GetRotation() const { return _worldRotation; }
  //!
  //! @brief Get the local rotation of the object (relative to parent)
  //!
  glm::quat GetLocalRotation() const { return _localRotation; }
  //!
  //! @brief Set the local rotation. Propagates world transform to children.
  //!
  void SetRotation(glm::quat rot) {
    _localRotation = rot;
    UpdateWorldTransform();
  }

  void GetParentWorld(glm::vec3 &pos, glm::quat &rot) const;
  void SetWorldPosition(glm::vec3 world);
  void AddWorldPosition(glm::vec3 delta) {
    SetWorldPosition(_worldPosition + delta);
  }
  void SetWorldRotation(glm::quat world);
  bool inheritParentRotation = false;
  glm::vec3 scale = {1, 1, 1};
  Object(std::string path, const std::shared_ptr<Scene> &scene);

  Object();
  Object(const std::shared_ptr<Scene> &scene);
  std::string _name;
  //!
  //! @brief Calls EarlySetup on components
  //!
  //!
  void EarlySetup();
  //!
  //! @brief Calls setup on components
  //!
  //!
  void Setup();
  //!
  //! @brief Calls update on components
  //!
  //!
  void Update();
  //!
  //! @brief Calls fixedUpdate on components
  //!
  //!
  void FixedUpdate();

  //!
  //! @brief Call end on all components
  //!
  //!
  void End();
  //!
  //! @brief Calls LateEnd on components
  //!
  //!
  void LateEnd();
  void Save();
  void Load();
  json ToJson() override;
  void FromJson(json &js) override;
  std::shared_ptr<Graphics::CameraComponent> FromJson(json &js, void *nullpt);
  std::shared_ptr<Graphics::CameraComponent> FromJson(json &js,
                                                      std::string path);
  std::weak_ptr<Scene> scene;
  std::weak_ptr<SceneObject> sceneobject;
  void fromParams(std::string name,
                  const std::vector<std::shared_ptr<IComponent>> &comps = {},
                  std::vector<float> position = {0, 0, 0},
                  std::vector<float> rotation = {1, 0, 0, 0},
                  glm::vec3 scale = {1, 1, 1});
  //!
  //! @brief Adds component to object unless already present
  //!
  //!
  template <typename T = IComponent> void AddComponent() {
    std::string key(T::GetNameS());
    AddComponent(key);
  }
  //!
  //! @brief Adds component to object unless already present
  //!
  //! @param[in] key key
  //!
  void AddComponent(const std::string &key) {
    ZoneScoped;
    if (_components.contains(key)) {
      SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER,
                         "Tried adding already present component");
      return;
    }
    auto component = Main::ScriptSystem::instance->GetScript(key);
    if (component == nullptr) {
      SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                          "Getscript returned nullptr for {}", key);
      return;
    }
    component->SetObject(shared_from_this());
    component->EarlySetup();
    component->Setup();
    _components[key] = component;
  }
  //!
  //! @brief Removes component from object unless not present
  //!
  //!
  template <typename T = IComponent> void RemoveComponent() {
    std::string key(T::GetNameS());
    RemoveComponent(key);
  }
  //!
  //! @brief Removes component from object unless not present
  //!
  //! @param[in] key key
  //!
  void RemoveComponent(const std::string &key) {
    if (!_components.contains(key)) {
      SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER,
                         "Tried to remomve component not present");
      return;
    }
    auto component = GetComponent(key);
    component->End();
    component->LateEnd();
    _components.erase(_components.find(key));
  }
  //!
  //! @brief Return the component specified if present, otherwise nullptr
  //!
  //!
  template <typename T = IComponent> std::shared_ptr<T> GetComponent() {
    std::string key(T::GetNameS());
    if (!_components.contains(key))
      return nullptr;
    return std::static_pointer_cast<T>(_components[key]);
  }
  //!
  //! @brief Return the component specified if present, otherwise nullptr
  //!
  //! @param[in] key key
  //!
  std::shared_ptr<IComponent> GetComponent(const std::string &key) {
    if (!_components.contains(key))
      return nullptr;
    return _components[key];
  }
  // TODO: Figure out why broken
  //!
  //! @brief Returns whether all components are valid
  //!
  //! @return .
  //!
  bool IsValid() {
    return std::ranges::all_of(
        _components, [](const auto &pair) { return pair.second->IsValid(); });
  }

  //!
  //! @brief Recomputes world position and rotation from parent transform, then
  //! signals children to do the same.
  //!
  void UpdateWorldTransform();

private:
  //!
  //! @_localPosition Position relative to parent (or world if no parent)
  //!
  glm::vec3 _localPosition = {0, 0, 0};
  //!
  //! @_worldPosition Cached world-space position (parent world pos + parent rot
  //! * local pos)
  //!
  glm::vec3 _worldPosition = {0, 0, 0};
  //!
  //! @_localRotation Rotation relative to parent (or world if no parent)
  //!
  glm::quat _localRotation = {1, 0, 0, 0};
  //!
  //! @_worldRotation Cached world-space rotation (parent world rot * local rot)
  //!
  glm::quat _worldRotation = {1, 0, 0, 0};
};
} // namespace Engine
