#pragma once
#include "Util/LoggerUtil.hpp"
#include "spdlog/spdlog.h"
#include <memory>
#define JSON_TRY_USER if (true)
#define JSON_CATCH_USER(exception) if (false)
#define JSON_THROW_USER(exception)                                             \
  {                                                                            \
                                                                               \
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,                                    \
                        "Error in {} : {}  (function {}) - {} ", __FILE__,     \
                        __LINE__, __FUNCTION__, (exception).what());           \
    std::abort();                                                              \
  }

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

struct JsonFileBase {
public:
  ObjectType object_type;
  json data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JsonFileBase, object_type, data)
} // namespace Engine
