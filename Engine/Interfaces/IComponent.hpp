#pragma once

#include "IJson.hpp"
#include "JsonFileBase.hpp"
#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>

#ifndef ENGINE_CUSTOM_COMPONENT_TYPES
#define ENGINE_CUSTOM_COMPONENT_TYPES_expand
#else
#define ENGINE_CUSTOM_COMPONENT_TYPES_expand , ENGINE_CUSTOM_COMPONENT_TYPES
#endif

using json = nlohmann::json;
namespace Engine {
class Object;
enum ENGINE_COMPONENT_TYPE {
  renderable,
  camera,
  ENGINE_CUSTOM_COMPONENT_TYPES_expand
};
struct ComponentJson {
public:
  ObjectType object_type;
  ENGINE_COMPONENT_TYPE type;
  json data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ComponentJson, object_type, type, data);
class IComponent : public IJson {
public:
  virtual ~IComponent() = default;
  virtual void Setup() = 0;
  virtual void Update() = 0;
  virtual void FixedUpdate() = 0;
  virtual void Save() = 0;
  virtual void Load() = 0;
  virtual ENGINE_COMPONENT_TYPE GetType() = 0;
  std::weak_ptr<Object> object;
  //  static std::map<ENGINE_COMPONENT_TYPE, void(json &js)> constructors;
};
} // namespace Engine
