#pragma once

#include <meta>
#include <vector>

#include "Interfaces/IComponent.hpp"
#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace Engine::Attr {
struct Script {}; // may be registered / added in editor
struct Serialize {
  const char *name = nullptr;
}; // override JSON key
struct Edit {};
struct Range {
  float min, max, speed = 0.1f;
};
struct ReadOnly {};

template <class T, class A> consteval auto fields_with() {
  std::vector<std::meta::info> v;
  for (auto m : std::meta::nonstatic_data_members_of(
           ^^T, std::meta::access_context::current()))
    if (!std::meta::annotations_of_with_type(m, ^^A).empty())
      v.push_back(m);
  return std::define_static_array(v);
}

consteval std::string_view json_key(std::meta::info m) {
  auto s = std::meta::extract<Attr::Serialize>(
      std::meta::annotations_of_with_type(m, ^^Attr::Serialize)[0]);
  return s.name ? std::string_view(s.name) : std::meta::identifier_of(m);
}

template <class T> struct ReflectedComponent : public Engine::IComponent {
  json ToJson() override {
    json j;
    auto &self = static_cast<T &>(*this);
    template for (constexpr auto m : fields_with<T, Attr::Serialize>())
      j[std::string(json_key(m))] = ReflectAccess::ref<m>(self);
    return j;
  }
  void FromJson(json &js) override {
    auto &self = static_cast<T &>(*this);
    template for (constexpr auto m : fields_with<T, Attr::Serialize>()) {
      std::string k(json_key(m));
      if (js.contains(k))
        js.at(k).get_to(ReflectAccess::ref<m>(self)); 
    }
  }
};
} // namespace Engine::Attr
