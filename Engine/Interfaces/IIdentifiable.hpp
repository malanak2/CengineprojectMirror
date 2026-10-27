#pragma once
#include <meta>
#include <string_view>
namespace Engine {
    struct ReflectAccess {
      template <std::meta::info M, class T>
      static auto &ref(T &o) { return o.[:M:]; }
    };
#define REGISTER_CLASS(name)                                                   \
  friend struct ::Engine::ReflectAccess;                                       \
public:                                                                        \
  std::string_view GetName() override { return #name; }                        \
  static std::string_view GetNameS() { return #name; }                         \
                                                                               \
private:\


class IIdentifiable {
public:
  virtual std::string_view GetName() = 0;
};
} // namespace Engine
