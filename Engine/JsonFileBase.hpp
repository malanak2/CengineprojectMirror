#pragma once
#include "Util/LoggerUtil.hpp"
#include "spdlog/spdlog.h"
#include "nlohmann/json.hpp" // IWYU pragma: keep
using json = nlohmann::json;

namespace Engine {
enum class ObjectType { Component, Object, Material, Shader, Texture, Invalid };
NLOHMANN_JSON_SERIALIZE_ENUM(ObjectType,
                             {
                                 {ObjectType::Component, "component"},
                                 {ObjectType::Object, "object"},
                                 {ObjectType::Material, "material"},
                                 {ObjectType::Shader, "shader"},
                                 {ObjectType::Texture, "texture"},
                                 {ObjectType::Invalid, nullptr},
                             });

//!
//! struct JsonFileBase - Used for saving objects and ensuring their type is
//! correct when loading
//!
struct JsonFileBase {
public:
  ObjectType object_type;
  json data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JsonFileBase, object_type, data)
} // namespace Engine
