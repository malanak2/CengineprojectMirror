#pragma once

#include "JsonFileBase.hpp"
#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>

#ifndef ENGINE_CUSTOM_COMPONENT_TYPES
#define ENGINE_CUSTOM_COMPONENT_TYPES_expand
#else
#define ENGINE_CUSTOM_COMPONENT_TYPES_expand , ENGINE_CUSTOM_COMPONENT_TYPES
#endif

using json = nlohmann::json;

enum ENGINE_COMPONENT_TYPE { renderable ENGINE_CUSTOM_COMPONENT_TYPES_expand };
struct ComponentJson {
public:
  ObjectType object_type;
  ENGINE_COMPONENT_TYPE type;
  json data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ComponentJson, object_type, type, data);
class IComponent {
public:
  virtual ~IComponent() = default;
  virtual void Setup() = 0;
  virtual void Update() = 0;
  virtual void FixedUpdate() = 0;
  virtual void Save() = 0;
  virtual void Load() = 0;
  virtual nlohmann::json ToJson() = 0;
  virtual ENGINE_COMPONENT_TYPE GetType() = 0;
};
