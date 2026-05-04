#pragma once

#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace Engine {
enum class ObjectType { Component, Object, Material, Shader, Invalid };
NLOHMANN_JSON_SERIALIZE_ENUM(ObjectType,
                             {
                                 {ObjectType::Component, "component"},
                                 {ObjectType::Object, "object"},
                                 {ObjectType::Material, "material"},
                                 {ObjectType::Shader, "shader"},
                                 {ObjectType::Invalid, nullptr},
                             });

struct JsonFileBase {
public:
  ObjectType object_type;
  json data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JsonFileBase, object_type, data)
} // namespace Engine
