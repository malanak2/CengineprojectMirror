#pragma once
#include "Interfaces/IComponent.hpp"
#include <map>
#include <memory>
#include <typeindex>
#include "Util/GlmJson.hpp"
// Attributes made with help of AI
namespace Engine
{
  namespace Attr
  {
    template <std::size_t N = 64>
    struct FixedString
    {
      char buf[N]{};

      constexpr FixedString() = default;

      constexpr FixedString(const char* str)
      {
        if (str)
        {
          for (std::size_t i = 0; i < N - 1 && str[i] != '\0'; ++i)
            buf[i] = str[i];
        }
      }

      constexpr FixedString(std::string_view sv)
      {
        std::size_t count = std::min(sv.size(), N - 1);
        for (std::size_t i = 0; i < count; ++i)
          buf[i] = sv[i];
      }

      constexpr operator std::string_view() const
      {
        return std::string_view{buf};
      }

      constexpr bool empty() const { return buf[0] == '\0'; }
    };

    struct Script
    {
      FixedString<64> name{};
      FixedString<64> displayname{};
    };
    struct Serialize
    {
      FixedString<64> name={};
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
    
    struct Editor {FixedString<64> displayname={};};
  }

  namespace Main
  {
    //!
    //! @def REGISTER_SCRIPT(script) Registers the @script specified into the system
    //!
#define REGISTER_SCRIPT(script)                                                \
  {                                                                            \
    (Engine::Main::ScriptSystem::instance->RegisterScriptA<script>());          \
  }
    
    template <class T, class A> 
    static consteval auto fields_with() {
      std::vector<std::meta::info> v;
      for (auto m : std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::unchecked())) {
        if (!std::meta::annotations_of_with_type(m, ^^A).empty()) v.push_back(m);
      }
      return std::define_static_array(v);
    }

    static consteval std::string_view json_key(std::meta::info m) {
      auto annos = std::meta::annotations_of_with_type(m, ^^Attr::Serialize);
      if (annos.empty()) return std::meta::identifier_of(m);
      auto s = std::meta::extract<Attr::Serialize>(annos[0]);
      return s.name.empty() ? std::meta::identifier_of(m) : std::string_view(s.name);
    }
    
    struct ScriptInfo
    {
      std::string_view name;
      std::string_view displayname;
      std::type_index  type;
      std::shared_ptr<IComponent> (*create)();
      
      json (*toJson)(IComponent*);
      void (*fromJson)(IComponent*, json&);
    };
    //!
    //! @brief System for registering user scripts.
    //!
    class ScriptSystem
    {
    public:
      static std::shared_ptr<ScriptSystem> instance;

      
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
      static consteval Attr::Script ScriptAttrOf_Impl()
      {
        Attr::Script out{};
        auto annos = std::meta::annotations_of_with_type(^^T, ^^Attr::Script);
        if (!annos.empty())
          out = std::meta::extract<Attr::Script>(annos[0]);
        std::string_view id = std::meta::identifier_of(^^T);
        if (out.name.empty())        out.name = id;
        if (out.displayname.empty()) out.displayname = out.name;
        return out;
      }

      template <typename T>
      static constexpr const Attr::Script& ScriptAttrOf()
      {
        // Caches the result in permanent static memory
        static constexpr Attr::Script instance = ScriptAttrOf_Impl<T>();
        return instance;
      }
      template <typename T>
            static constexpr std::string_view NameOf() 
      { 
        static constexpr Attr::Script attr = ScriptAttrOf<T>();
        return attr.name; 
      }      
      template <typename T>
    void RegisterScriptA()
      {
        static constexpr Attr::Script attr = ScriptAttrOf<T>();

        // Lambda that merges manual ToJson() with Reflected fields
        auto toJsonWrapper = [](IComponent* ptr) -> json {
          T* self = static_cast<T*>(ptr);
        
          // 1. Run the custom virtual ToJson() from the component
          json j = self->ToJson(); 
        
          // 2. Append the reflected fields
          template for (constexpr auto m : fields_with<T, Attr::Serialize>()) {
            j[std::string(json_key(m))] = self->[:m:];
          }
          return j;
        };

        // Lambda that merges manual FromJson() with Reflected fields
        auto fromJsonWrapper = [](IComponent* ptr, json& js) {
          T* self = static_cast<T*>(ptr);
        
          // 1. Extract the reflected fields
          template for (constexpr auto m : fields_with<T, Attr::Serialize>()) {
            std::string k(json_key(m));
            if (js.contains(k)) js.at(k).get_to(self->[:m:]);
          }
        
          // 2. Run the custom virtual FromJson() from the component
          self->FromJson(js); 
        };

        auto [it, inserted] = by_type.try_emplace(
            std::type_index(typeid(T)),
            ScriptInfo{attr.name, attr.displayname, std::type_index(typeid(T)),
                       &make<T>, toJsonWrapper, fromJsonWrapper});
        
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
      template <std::meta::info Scope = ^^::>
    void create_global_registry()
    {
      scan_scope<Scope>();
    }

    static constexpr auto access_ctx = std::meta::access_context::unchecked();
      

  private:
    template <typename T>
    static std::shared_ptr<IComponent> make() { return std::make_shared<T>(); }

    static consteval bool has_script_annotation(std::meta::info m)
    {
      return !std::meta::annotations_of_with_type(m, ^^Attr::Script).empty();
    }

    std::unordered_map<std::type_index, ScriptInfo> by_type;
    std::unordered_map<std::string_view, const ScriptInfo*> by_name;
      std::vector<const ScriptInfo*> ordered;
  };
  } // namespace Main
} // namespace Engine
