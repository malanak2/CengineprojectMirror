#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class IJson {
  virtual json ToJson() = 0;
};
