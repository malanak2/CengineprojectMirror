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
#include <vector>
using json = nlohmann::json;

namespace Engine {
class Scene;
//!
//! struct ObjectJson - Json struct for saving objects
//!
struct ObjectJson {
  std::vector<float> position;
  std::vector<float> rotation;
  std::vector<ComponentJson> components;
  std::string name;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ObjectJson, position, rotation, components,
                                   name)
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
  //! @_position The position of the object
  //!
  glm::vec3 _position;
  // TODO:Quaternion support for La(e)rp
  //!
  //! @_rotation The rotation of the object
  //!
  glm::quat _rotation;
  Object(std::string path, std::shared_ptr<Scene> scene);
  Object(std::shared_ptr<Scene> scene);
  std::string _name;
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
  void Save();
  void Load();
  json ToJson() override;
  void FromJson(json &js) override;
  std::shared_ptr<Graphics::CameraComponent> FromJson(json &js, void *nullpt);
  std::shared_ptr<Graphics::CameraComponent> FromJson(json &js,
                                                      std::string path);
  std::weak_ptr<Scene> scene;
  void fromParams(std::string name,
                  std::vector<std::shared_ptr<IComponent>> comps = {},
                  std::vector<float> position = {0, 0, 0},
                  std::vector<float> rotation = {0, 0, 0, 0});
  //!
  //! @brief Adds component to object unless already present
  //!
  //!
  template <typename T = IComponent> void AddComponent() {
    std::string key = T::GetNameS();
    AddComponent(key);
  }
  //!
  //! @brief Adds component to object unless already present
  //!
  //! @param[in] key key
  //!
  void AddComponent(std::string key) {
    if (_components.contains(key)) {
      SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER,
                         "Tried adding already present component");
      return;
    }
    auto component = Main::ScriptSystem::instance->GetScript(key);
    component->SetObject(shared_from_this());
    component->Setup();
    _components[key] = component;
  }
  //!
  //! @brief Removes component from object unless not present
  //!
  //!
  template <typename T = IComponent> void RemoveComponent() {
    std::string key = T::GetNameS();
    RemoveComponent(key);
  }
  //!
  //! @brief Removes component from object unless not present
  //!
  //! @param[in] key key
  //!
  void RemoveComponent(std::string key) {
    if (!_components.contains(key)) {
      SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER,
                         "Tried to remomve component not present");
      return;
    }
    auto component = GetComponent(key);
    component->End();
    _components.erase(key);
  }
  //!
  //! @brief Return the component specified if present, otherwise nullptr
  //!
  //!
  template <typename T = IComponent> std::shared_ptr<T> GetComponent() {
    if (!_components.contains(T::GetNameS()))
      return nullptr;
    return _components[T::GetnameS()];
  }
  //!
  //! @brief Return the component specified if present, otherwise nullptr
  //!
  //! @param[in] key key
  //!
  std::shared_ptr<IComponent> GetComponent(std::string key) {
    if (!_components.contains(key))
      return nullptr;
    return _components[key];
  }
};
} // namespace Engine
