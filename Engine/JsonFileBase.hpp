#pragma once

#include "nlohmann/json.hpp"
#include <nlohmann/detail/macro_scope.hpp>
using json = nlohmann::json;

enum ObjectType { Component, Object, Material, Shader, Invalid };
NLOHMANN_JSON_SERIALIZE_ENUM(ObjectType, {
                                             {Component, "component"},
                                             {Object, "object"},
                                             {Material, "material"},
                                             {Shader, "shader"},
                                             {Invalid, nullptr},
                                         });

struct JsonFileBase {
public:
  ObjectType object_type;
  json data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JsonFileBase, object_type, data)
