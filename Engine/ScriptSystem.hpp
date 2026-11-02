#pragma once
#include "Interfaces/IComponent.hpp"
#include <map>
#include <memory>
#include <typeindex>

namespace Engine
{
  namespace Attr
  {
    struct Script
    {
      const char* name;
      const char* displayname;
    }; // may be registered / added in editor
    struct Serialize
    {
      const char* name = nullptr;
    }; // override JSON key
    struct Edit
    {
    };

    struct Range
    {
      float min, max, speed = 0.1f;
    };

    struct ReadOnly
    {
    };
    
    struct Editor {};
  }

  namespace Main
  {
    //!
    //! @def REGISTER_SCRIPT(script) Registers the @script specified into the system
    //!
#define REGISTER_SCRIPT(script)                                                \
  {                                                                            \
    (Engine::Main::ScriptSystem::instance->RegisterScript<script>());          \
  }
    
    struct ScriptInfo
    {
      std::string_view name;
      std::string_view displayname;
      std::type_index  type;
      std::shared_ptr<IComponent> (*create)();
    };
    //!
    //! @brief System for registering user scripts.
    //!
    class ScriptSystem
    {
    public:
      static std::shared_ptr<ScriptSystem> instance;

      template <typename T>
      void RegisterScript()
      {
        SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Registering script {}",
                           T::GetNameS());
        map_comp[std::string(T::GetNameS())] = []() -> std::shared_ptr<IComponent>
        {
          return std::make_shared<T>();
        };
      }

      template <typename T>
      void UnregisterScript()
      {
        SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Unregistering script {}",
                           T::GetNameS());
        map_comp.erase(std::string(T::GetNameS()));
      }

      std::shared_ptr<IComponent> GetScript(std::string_view key);

      std::map<std::string, std::function<std::shared_ptr<IComponent>()>,
               std::less<>>
      map_comp;
      static inline std::unordered_map<std::string_view,
                                 std::function<std::shared_ptr<IComponent>()>> map;
    private:
      static consteval bool is_script_candidate(std::meta::info m)
      {
        return std::meta::is_class_type(m)
            && std::meta::is_complete_type(m)
            && !std::meta::is_abstract_type(m)
            && !std::meta::is_same_type(m, ^^IComponent)
            && std::meta::is_base_of_type(^^IComponent, m);
      }

      template <std::meta::info Scope>
      static consteval auto members() {
        return std::define_static_array(std::meta::members_of(Scope, access_ctx));
      }

      template <std::meta::info Member>
      static consteval auto annotations() {
        return std::define_static_array(std::meta::annotations_of(Member));
      }

    public:
      
      template <typename T>
          static consteval Attr::Script ScriptAttrOf()
      {
        Attr::Script out{};
        for (auto a : std::meta::annotations_of(^^T))
          if (std::meta::type_of(a) == ^^Attr::Script)
            out = std::meta::extract<Attr::Script>(a);

        const char* id = std::define_static_string(std::meta::identifier_of(^^T));
        if (!out.name)        out.name = id;
        if (!out.displayname) out.displayname = out.name;
        return out;
      }
      
      template <typename T>
          static constexpr std::string_view NameOf() { return ScriptAttrOf<T>().name; }
      
template <typename T>
    void RegisterScriptA()
    {
      constexpr Attr::Script attr = ScriptAttrOf<T>();

      auto [it, inserted] = by_type.try_emplace(
          std::type_index(typeid(T)),
          ScriptInfo{attr.name, attr.displayname, std::type_index(typeid(T)),
                     &make<T>});
        if (inserted)
        {
          by_name[it->second.name] = &it->second;
          ordered.push_back(&it->second);
        }
    }

    template <typename T>
    void UnregisterScriptA()
    {
      auto it = by_type.find(std::type_index(typeid(T)));
      if (it == by_type.end()) return;
      by_name.erase(it->second.name);
        
        std::erase(ordered, &it->second);
      by_type.erase(it);
    }
      std::span<const ScriptInfo* const> All() const { return ordered; }
      
      template <typename F>
      void ForEach(F&& f) const
      {
        for (const ScriptInfo* info : ordered)
          f(*info);
      }

      auto begin() const { return ordered.begin(); }
      auto end()   const { return ordered.end(); }

    template <typename T>
    const ScriptInfo* GetScript() const
    {
      auto it = by_type.find(std::type_index(typeid(T)));
      return it == by_type.end() ? nullptr : &it->second;
    }

    template <typename T>
    std::shared_ptr<T> Create() const
    {
      if (auto* info = GetScript<T>())
        return std::static_pointer_cast<T>(info->create());
      return nullptr;
    }

    const ScriptInfo* GetScript(std::string_view name) const
    {
      auto it = by_name.find(name);
      return it == by_name.end() ? nullptr : it->second;
    }

    std::shared_ptr<IComponent> Create(std::string_view name) const
    {
      auto* info = GetScript(name);
      return info ? info->create() : nullptr;
    }

    template <std::meta::info Scope>
    void scan_scope()
    {
      template for (constexpr auto member : members<Scope>())
      {
        if constexpr (std::meta::is_namespace(member))
          scan_scope<member>();
        else if constexpr (std::meta::is_type(member) && is_script_candidate(member))
        {
          if constexpr (has_script_annotation(member))
            RegisterScriptA<typename [:member:]>();
        }
      }
    }

    void create_global_registry()
    {
      scan_scope<^^::>();
    }

    static constexpr auto access_ctx = std::meta::access_context::unchecked();
      

  private:
    template <typename T>
    static std::shared_ptr<IComponent> make() { return std::make_shared<T>(); }

    static consteval bool has_script_annotation(std::meta::info m)
    {
      for (auto a : std::meta::annotations_of(m))
        if (std::meta::type_of(a) == ^^Attr::Script) return true;
      return false;
    }

    std::unordered_map<std::type_index, ScriptInfo> by_type;
    std::unordered_map<std::string_view, const ScriptInfo*> by_name;
      std::vector<const ScriptInfo*> ordered;
  };
    /*
      template <std::meta::info Scope>
      void scan_scope()
      {
        template for (constexpr auto member : members<Scope>())
        {
          if constexpr (std::meta::is_namespace(member))
          {
            scan_scope<member>();
          }
          else if constexpr (std::meta::is_type(member) && is_script_candidate(member))
          {
            template for (constexpr auto anno : annotations<member>())
            {
              if constexpr (std::meta::type_of(anno) == ^^Attr::Script)
              {
                constexpr auto attr = std::meta::extract<Attr::Script>(anno);

                constexpr std::string_view name = attr.name
                    ? attr.name
                    : std::define_static_string(std::meta::identifier_of(member));

                map[name] = []() -> std::shared_ptr<IComponent> {
                  return std::make_shared<typename [:member:]>();
                };
              }
            }
          }
        }
      }
      static constexpr auto access_ctx = std::meta::access_context::unchecked();
      void create_global_registry() {
        scan_scope<^^::>();
      }
    };*/
  } // namespace Main
} // namespace Engine
