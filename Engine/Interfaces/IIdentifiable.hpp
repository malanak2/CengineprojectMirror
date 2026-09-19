#pragma once

#include <string_view>
namespace Engine {
#define REGISTER_CLASS(name)                                                   \
public:                                                                        \
  std::string_view GetName() override { return #name; }                        \
  static std::string_view GetNameS() { return #name; }                         \
                                                                               \
private:

class IIdentifiable {
public:
  virtual std::string_view GetName() = 0;
};
} // namespace Engine
