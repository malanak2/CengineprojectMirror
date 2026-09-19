#pragma once

#include "Graphics/Components/CameraComponent.hpp"
#include "Graphics/Components/ComponentRenderable.hpp"
#include "Interfaces/IComponent.hpp"
#include <memory>
#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>
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
  glm::vec3 _rotation;
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
};
} // namespace Engine
