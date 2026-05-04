#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace Engine {
class IJson {
public:
  virtual ~IJson() = default;
  virtual json ToJson() = 0;
  virtual void FromJson(json &js) = 0;
};
} // namespace Engine
