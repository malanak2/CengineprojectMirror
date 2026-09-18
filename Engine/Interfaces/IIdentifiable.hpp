#pragma once

#include <string_view>
namespace Engine {
#define REGISTER_CLASS(name)                                                   \
  std::string_view GetName() override { return #name; }                        \
  static std::string_view GetNameS() { return #name; }

class IIdentifiable {
public:
  virtual std::string_view GetName() = 0;
};
} // namespace Engine
