#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class IJson {
public:
  virtual json ToJson() = 0;
  virtual void FromJson(json &js) = 0;
};
