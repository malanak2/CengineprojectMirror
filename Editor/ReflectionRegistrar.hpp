#pragma once
#include <imgui.h>
#include <meta>

#include "ScriptSystem.hpp"

class ReflectionRegistrar
{
    struct FieldOpts
    {
      bool  readOnly = false;
      bool  hasRange = false;
      float min = 0, max = 0, speed = 0.1f;
      Engine::Attr::FixedString<64> display_name = {};
    };

    static consteval FieldOpts OptsOf(std::meta::info m)
    {
      FieldOpts o;
      o.readOnly =
        !std::meta::annotations_of_with_type(m, ^^Engine::Attr::ReadOnly).empty();
      auto e = std::meta::annotations_of_with_type(m, ^^Engine::Attr::Editor);
      if (!e.empty())
      {
        auto v = std::meta::extract<Engine::Attr::Editor>(e[0]);
        if (v.displayname.empty())
        {
          o.display_name = std::define_static_string(std::meta::identifier_of(m));
        } else {
          o.display_name = v.displayname;
        }
      }
      auto r = std::meta::annotations_of_with_type(m, ^^Engine::Attr::Range);
      if (!r.empty())
      {
        auto v = std::meta::extract<Engine::Attr::Range>(r[0]);
        o.hasRange = true;
        o.min = v.min;
        o.max = v.max;
        o.speed = v.speed;
      }
      return o;
    }

    template <class F>
    static void DrawField(const char* label, F& v, const FieldOpts& o)
    {
      if constexpr (std::is_same_v<F, bool>)
        ImGui::Checkbox(label, &v);
      else if constexpr (std::is_same_v<F, float>)
      {
        if (o.hasRange) ImGui::DragFloat(label, &v, o.speed, o.min, o.max);
        else            ImGui::InputFloat(label, &v);
      }
      else if constexpr (std::is_same_v<F, double>)
        ImGui::InputDouble(label, &v);
      else if constexpr (std::is_same_v<F, int>)
      {
        if (o.hasRange) ImGui::DragInt(label, &v, o.speed, (int)o.min, (int)o.max);
        else            ImGui::InputInt(label, &v);
      }
      else if constexpr (std::is_same_v<F, std::string>)
      {
        char buf[256];
        strncpy(buf, v.c_str(), sizeof(buf));
        buf[sizeof(buf) - 1] = 0;
        if (ImGui::InputText(label, buf, sizeof(buf))) v = buf;
      }
      else if constexpr (std::is_same_v<F, glm::vec2>)
        ImGui::InputFloat2(label, &v[0]);
      else if constexpr (std::is_same_v<F, glm::vec3>)
        ImGui::InputFloat3(label, &v[0]);
      else if constexpr (std::is_same_v<F, glm::vec4>)
        ImGui::InputFloat4(label, &v[0]);
      else if constexpr (std::is_same_v<F, glm::quat>)
      {
        glm::vec3 deg = glm::degrees(glm::eulerAngles(v));
        if (ImGui::DragFloat3(label, &deg[0], 1.0f))
          v = glm::normalize(glm::quat(glm::radians(deg)));
      }
      else
        ImGui::TextDisabled("%s: unsupported type", label);
    }

    template <class T>
    static void DrawReflected(Engine::IComponent* c)
    {
      T* self = static_cast<T*>(c);
      template for (constexpr auto m :
                    Engine::Main::fields_with<T, Engine::Attr::Editor>())
      {
        constexpr FieldOpts opts = OptsOf(m);
        
        const char* label =
          &opts.display_name.buf[0];

        ImGui::PushID(label);
        if (opts.readOnly) ImGui::BeginDisabled();
        DrawField(label, self->[:m:], opts);
        if (opts.readOnly) ImGui::EndDisabled();
        ImGui::PopID();
      }
    }
  
public:
    struct FieldMeta {
    const char* name; const char* type; const char* jsonKey = nullptr;
    size_t size = 0, offset = 0;
    bool isPublic = false, editor = false, serialize = false,
         readOnly = false, hasRange = false, drawable = false;
    float min = 0, max = 0, speed = 0;
  };
  struct ComponentMeta {
    std::string_view name, displayName;
    const char* typeName = "";
    size_t size = 0;
    std::vector<FieldMeta> fields;
  };
  std::vector<ComponentMeta> meta;

  bool HasReflected(std::string_view n) const { return reflected.contains(n); }

  template <class F>
  static constexpr bool IsDrawable =
      std::is_same_v<F, bool> || std::is_same_v<F, float> ||
      std::is_same_v<F, double> || std::is_same_v<F, int> ||
      std::is_same_v<F, std::string> || std::is_same_v<F, glm::vec2> ||
      std::is_same_v<F, glm::vec3> || std::is_same_v<F, glm::vec4> ||
      std::is_same_v<F, glm::quat>;

  static consteval bool HasAnno(std::meta::info m, std::meta::info a) {
    return !std::meta::annotations_of_with_type(m, a).empty();
  }

  template <class T>
  void RegisterMeta() {
    constexpr auto ctx = std::meta::access_context::unchecked();
    ComponentMeta cm;
    cm.name = Engine::Main::ScriptSystem::NameOf<T>();
    cm.displayName = Engine::Main::ScriptSystem::ScriptAttrOf<T>().displayname;
    cm.typeName = std::define_static_string(std::meta::display_string_of(^^T));
    cm.size = sizeof(T);

    template for (constexpr auto m : std::define_static_array(
                      std::meta::nonstatic_data_members_of(^^T, ctx))) {
      using F = typename [:std::meta::type_of(m):];
      constexpr FieldOpts o = OptsOf(m);
      constexpr bool ser = HasAnno(m, ^^Engine::Attr::Serialize);
      constexpr const char* key =
          ser ? std::define_static_string(Engine::Main::json_key(m)) : nullptr;

      FieldMeta f;
      f.name = std::define_static_string(std::meta::identifier_of(m));
      f.type = std::define_static_string(
          std::meta::display_string_of(std::meta::type_of(m)));
      f.jsonKey = key;
      f.size = sizeof(F);
      f.offset = std::meta::offset_of(m).bytes;
      f.isPublic = std::meta::is_public(m);
      
constexpr bool isEditor = HasAnno(m, ^^Engine::Attr::Editor);
      f.editor = isEditor;
      f.serialize = ser;
      f.readOnly = o.readOnly;
      f.hasRange = o.hasRange;
      f.min = o.min; f.max = o.max; f.speed = o.speed;
      f.drawable = IsDrawable<F>;
      cm.fields.push_back(f);
    }
    meta.push_back(std::move(cm));
  }
  static void Init();
  void Register(std::string_view type,
                void (*func)(const std::shared_ptr<Engine::IComponent>&));
  void Render(const std::shared_ptr<Engine::IComponent>& uni);

  template <class T>
  void RegisterReflected()
  {
    constexpr auto fields =
      Engine::Main::fields_with<T, Engine::Attr::Editor>();
    if constexpr (fields.size() > 0)
      reflected.try_emplace(Engine::Main::ScriptSystem::NameOf<T>(),
                            &DrawReflected<T>);
  }

  template <std::meta::info Scope>
  void RegisterReflectedScope()
  {
    constexpr auto ctx = std::meta::access_context::unchecked();
    template for (constexpr auto member :
                  std::define_static_array(std::meta::members_of(Scope, ctx)))
    {
      if constexpr (std::meta::is_namespace(member))
        RegisterReflectedScope<member>();
      else if constexpr (std::meta::is_type(member)
                      && std::meta::is_class_type(member)
                      && std::meta::is_complete_type(member)
                      && !std::meta::is_abstract_type(member)
                      && !std::meta::is_same_type(member, ^^Engine::IComponent)
                      && std::meta::is_base_of_type(^^Engine::IComponent, member)
                      && !std::meta::annotations_of_with_type(
                            member, ^^Engine::Attr::Script).empty()) {
        RegisterMeta<typename [:member:]>();
        RegisterReflected<typename [:member:]>();
                            }
    }
  }

  static ReflectionRegistrar& Get() {
    static ReflectionRegistrar inst;
    return inst;
  }
  std::map<std::string_view, void (*)(Engine::IComponent*)> reflected;
};
