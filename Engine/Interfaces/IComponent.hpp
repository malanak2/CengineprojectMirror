#pragma once

#include "IJson.hpp"
#include "Interfaces/IIdentifiable.hpp"
#include "JsonFileBase.hpp"
#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace Engine {
class Object;
struct ComponentJson {
public:
  std::string name;
  json data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ComponentJson, name, data);
//!
//! @brief Default interface for components
//!
class IComponent : public IJson, public IIdentifiable {
public:
  virtual ~IComponent() = default;
  //!
  //! @brief Called on setup
  //!
  //!
  virtual void Setup();
  //!
  //! @brief Called every frame
  //!
  //!
  virtual void Update();
  //!
  //! @brief Called every fixedUpdate
  //!
  //!
  virtual void FixedUpdate();
  //!
  //! @brief Called when component is removed / on program end
  //!
  //!
  virtual void End();

  float GetDeltaTime();

  void SetObject(std::shared_ptr<Object> object);

  //!
  //! @object Pointer to the object this is assigned to
  //!
  std::weak_ptr<Object> object;
  //  static std::map<ENGINE_COMPONENT_TYPE, void(json &js)> constructors;
};
} // namespace Engine
