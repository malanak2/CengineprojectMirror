#pragma once

#include "IJson.hpp"
#include "JsonFileBase.hpp"
#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace Engine {
class Object;
enum ENGINE_COMPONENT_TYPE {
  renderable,
  camera,
  script,
};
struct ComponentJson {
public:
  ObjectType object_type;
  ENGINE_COMPONENT_TYPE type;
  json data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ComponentJson, object_type, type, data);
//!
//! @brief Default interface for components
//!
class IComponent : public IJson {
public:
  virtual ~IComponent() = default;
  //!
  //! @brief Called on setup
  //!
  //!
  virtual void Setup() = 0;
  //!
  //! @brief Called every frame
  //!
  //!
  virtual void Update() = 0;
  //!
  //! @brief Called every fixedUpdate
  //!
  //!
  virtual void FixedUpdate() = 0;
  // TODO: I dont think save and load should be void... What was I doing here?
  virtual void Save() = 0;
  virtual void Load() = 0;

  virtual ENGINE_COMPONENT_TYPE GetType() = 0;
  // TODO: Remove
  virtual void RenderImGui() = 0;
  //!
  //! @brief Name of the component
  //!
  //!
  virtual std::string GetName() = 0;

  //!
  //! @object Pointer to the object this is assigned to
  //!
  std::weak_ptr<Object> object;
  //  static std::map<ENGINE_COMPONENT_TYPE, void(json &js)> constructors;
};
} // namespace Engine
