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
  //! @brief Pre-Setup, allways runs
  //!
  //!
  virtual void EarlySetup();
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
  //!
  //! @brief Post-End, allways runs
  //!
  //!
  virtual void LateEnd();

  virtual float GetDeltaTime();

  virtual void SetObject(const std::shared_ptr<Object> &object);

  //!
  //! @object Pointer to the object this is assigned to
  //!
  std::weak_ptr<Object> object;
  //!
  //! @brief Returns if follows paused execution. This is intended to only
  //! really be used in Editor stuff - camera etc.
  //!
  //! @return wheter follows
  //!
  virtual bool FollowsPausedExecution() { return true; }

  //!
  //! @brief Returns whether a component is valid (whether any errors were
  //! encountered)
  //!
  //! @return .
  //!
  virtual bool IsValid() { return true; }
};
} // namespace Engine
