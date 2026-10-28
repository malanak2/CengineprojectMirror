#pragma once

#include <meta>
#include <vector>

#include "Interfaces/IComponent.hpp"
#include "nlohmann/json.hpp"
using json = nlohmann::json;

#define COMPONENT(compname) \
class [[=Attr::Script{.name = #compname}]] compname : public Attr::ReflectedComponent<compname>

namespace Engine::Attr {

struct Script {const char* name;const char* displayname;}; // may be registered / added in editor
struct Serialize {
  const char *name = nullptr;
}; // override JSON key
struct Edit {};
struct Range {
  float min, max, speed = 0.1f;
};
struct ReadOnly {};
  class ScriptService {
  public:
    struct ScriptInfo {
      std::string_view name;
      std::string_view displayname;
      IComponent* (*factory)();
    };

    static void register_script(ScriptInfo info) {
      registry().push_back(info);
    }

    static const std::vector<ScriptInfo>& get_scripts() {
      return registry();
    }

  private:
    static std::vector<ScriptInfo>& registry() {
      static std::vector<ScriptInfo> instance;
      return instance;
    }
  };
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

template <class T> struct ReflectedComponent : public ::Engine::IComponent {
private:
  static consteval Script extract_script_attr() {
    constexpr std::meta::info type_refl = ^^T;
    for (std::meta::info ann : std::meta::annotations_of(type_refl)) {
      if (std::meta::type_of(ann) == ^^Script) {
        return std::meta::extract<Script>(ann);
      }
    }
    return Script{}; 
  }

  inline static const bool registered = []() {
    constexpr Script attr = extract_script_attr();
    std::string_view script_name;
    if (attr.name != nullptr && std::string_view(attr.name).length() > 0) {
      script_name = attr.name;
    } else {
      script_name = std::meta::identifier_of(^^T);
    }
    
    std::string_view display_name;
    if (attr.displayname != nullptr && std::string_view(attr.displayname).length() > 0)
    {
      display_name = attr.displayname;
    } else
    {
      display_name = std::meta::identifier_of(^^T);
    }
    
    ScriptService::register_script({
        .name = script_name,
        .displayname = display_name,
        .factory = []() -> IComponent* { return new T(); }
    });
    return true;
  }();

  // Virtual anchor guarantees 'registered' variable isn't eliminated by dead-code stripping
  virtual void _anchor_reflection() { (void)registered; }
public:
  ReflectedComponent() { (void)registered; }
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
