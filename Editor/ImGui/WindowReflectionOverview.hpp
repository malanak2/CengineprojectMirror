#pragma once

// Editor/ImGui/WindowReflectionDebug.hpp
//
// Debug window for the whole reflection system:
//   * Overview   - counts + a list of problems (missing UI, unsupported types, ...)
//   * Components - every [[=Attr::Script]] component with all its fields and attributes
//   * Live       - JSON produced by ScriptInfo::toJson for the selected scene object
//   * Self-test  - create -> toJson -> fromJson -> toJson round-trip for every script

#include "Engine.hpp"
#include "ImGui/Window.hpp"
#include "ImGui/WindowScene.hpp"
#include "ImGuiRenderers.hpp"
#include "ReflectionRegistrar.hpp"
#include "ScriptSystem.hpp"
#include <algorithm>
#include <cctype>
#include <exception>
#include <imgui.h>
#include <memory>
#include <string>
#include <tracy/Tracy.hpp>
#include <vector>

namespace Editor {
class WindowReflectionDebug : public Window {
public:
  explicit WindowReflectionDebug(std::shared_ptr<WindowScene> sceneWindow)
      : sceneWindow(std::move(sceneWindow)) {}

  std::string GetName() override { return "Reflection Debug"; }

protected:
  std::shared_ptr<WindowScene> sceneWindow;

  char filter[128] = "";
  bool onlyProblems = false;

  struct SelfTestResult {
    std::string name;
    bool ok = false;
    std::string message;
  };
  std::vector<SelfTestResult> selfTest;

  static constexpr ImVec4 kRed{1.0f, 0.35f, 0.35f, 1.0f};
  static constexpr ImVec4 kYellow{1.0f, 0.85f, 0.3f, 1.0f};
  static constexpr ImVec4 kGreen{0.4f, 0.9f, 0.4f, 1.0f};
  static constexpr ImVec4 kGrey{0.6f, 0.6f, 0.6f, 1.0f};

  // ---------------------------------------------------------------- helpers
  static std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    return s;
  }

  bool matchesFilter(const ReflectionRegistrar::ComponentMeta &c) const {
    if (filter[0] == '\0')
      return true;
    auto f = lower(filter);
    return lower(std::string(c.name)).find(f) != std::string::npos ||
           lower(std::string(c.displayName)).find(f) != std::string::npos;
  }

  static void badge(const char *text, ImVec4 color) {
    ImGui::TextColored(color, "[%s]", text);
    ImGui::SameLine();
  }

  static void yesNo(bool v, const char *yes = "yes") {
    if (v)
      ImGui::TextColored(kGreen, "%s", yes);
    else
      ImGui::TextColored(kGrey, "-");
  }

  //! Everything the window knows about how a component shows up in the editor
  struct Status {
    bool reflected = false;   // has a generated Attr::Editor renderer
    bool manual = false;      // has an IMGUI_REGISTER_COMPONENT renderer
    int editorFields = 0;
    int unsupportedEditor = 0; // Attr::Editor field whose type has no widget
    bool noUi() const { return !reflected && !manual; }
    bool possibleDuplicate() const { return reflected && manual; }
    bool problem() const { return noUi() || unsupportedEditor > 0; }
  };

  static Status statusOf(const ReflectionRegistrar::ComponentMeta &c) {
    Status s;
    s.reflected = ReflectionRegistrar::Get().HasReflected(c.name);
    s.manual = ImGuiR::ImGuiComponentRenderer::instance->HasManual(c.name);
    for (const auto &f : c.fields) {
      if (f.editor) {
        s.editorFields++;
        if (!f.drawable)
          s.unsupportedEditor++;
      }
    }
    return s;
  }

  // ------------------------------------------------------------------- tabs
  void drawOverview() {
    const auto &metas = ReflectionRegistrar::Get().meta;
    auto *scripts = Engine::Main::ScriptSystem::instance.get();

    int withEditor = 0, noUi = 0, dup = 0, unsupported = 0, fields = 0,
        serialized = 0, editorFields = 0;
    for (const auto &c : metas) {
      auto s = statusOf(c);
      withEditor += s.editorFields > 0;
      noUi += s.noUi();
      dup += s.possibleDuplicate();
      unsupported += s.unsupportedEditor;
      fields += (int)c.fields.size();
      editorFields += s.editorFields;
      for (const auto &f : c.fields)
        serialized += f.serialize;
    }

    ImGui::Text("Scripts in ScriptSystem:      %zu", scripts->All().size());
    ImGui::Text("Components with metadata:     %zu", metas.size());
    ImGui::Text("Components with Editor fields: %d", withEditor);
    ImGui::Text("Total data members:           %d", fields);
    ImGui::Text("  Attr::Editor:               %d", editorFields);
    ImGui::Text("  Attr::Serialize:            %d", serialized);
    ImGui::Separator();

    // ScriptSystem <-> metadata consistency
    if (scripts->All().size() != metas.size())
      ImGui::TextColored(kRed,
                         "Mismatch: ScriptSystem has %zu scripts but %zu have "
                         "reflection metadata (was RegisterMeta<T> called?)",
                         scripts->All().size(), metas.size());

    ImGui::TextColored(noUi ? kRed : kGreen, "Components without any editor UI: %d",
                       noUi);
    ImGui::TextColored(unsupported ? kYellow : kGreen,
                       "Attr::Editor fields with unsupported type: %d", unsupported);
    ImGui::TextColored(dup ? kYellow : kGreen,
                       "Components with both manual + reflected UI: %d", dup);

    ImGui::Separator();
    ImGui::TextUnformatted("Problems");
    bool any = false;
    for (const auto &c : metas) {
      auto s = statusOf(c);
      if (s.noUi()) {
        any = true;
        ImGui::TextColored(kRed, "%s: no renderer (add Attr::Editor fields or "
                                 "IMGUI_REGISTER_COMPONENT)",
                           std::string(c.name).c_str());
      }
      for (const auto &f : c.fields)
        if (f.editor && !f.drawable) {
          any = true;
          ImGui::TextColored(kYellow, "%s::%s has unsupported type %s",
                             std::string(c.name).c_str(), f.name, f.type);
        }
      if (s.possibleDuplicate()) {
        any = true;
        ImGui::TextColored(kYellow,
                           "%s: manual renderer + %d reflected field(s) - "
                           "check for duplicate widgets",
                           std::string(c.name).c_str(), s.editorFields);
      }
    }
    if (!any)
      ImGui::TextColored(kGreen, "None");
  }

  void drawFieldTable(const ReflectionRegistrar::ComponentMeta &c) {
    constexpr ImGuiTableFlags flags =
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX;
    if (!ImGui::BeginTable("fields", 10, flags))
      return;
    ImGui::TableSetupColumn("Field");
    ImGui::TableSetupColumn("Type");
    ImGui::TableSetupColumn("Offset");
    ImGui::TableSetupColumn("Size");
    ImGui::TableSetupColumn("Access");
    ImGui::TableSetupColumn("Editor");
    ImGui::TableSetupColumn("Serialize");
    ImGui::TableSetupColumn("Range");
    ImGui::TableSetupColumn("ReadOnly");
    ImGui::TableSetupColumn("Widget");
    ImGui::TableHeadersRow();

    for (const auto &f : c.fields) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(f.name);
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(f.type);
      ImGui::TableNextColumn();
      ImGui::Text("%zu", f.offset);
      ImGui::TableNextColumn();
      ImGui::Text("%zu", f.size);
      ImGui::TableNextColumn();
      ImGui::TextColored(f.isPublic ? kGreen : kGrey,
                         f.isPublic ? "public" : "non-public");
      ImGui::TableNextColumn();
      yesNo(f.editor);
      ImGui::TableNextColumn();
      if (f.serialize)
        ImGui::TextColored(kGreen, "\"%s\"", f.jsonKey);
      else
        ImGui::TextColored(kGrey, "-");
      ImGui::TableNextColumn();
      if (f.hasRange)
        ImGui::Text("%.2f .. %.2f (x%.2f)", f.min, f.max, f.speed);
      else
        ImGui::TextColored(kGrey, "-");
      ImGui::TableNextColumn();
      yesNo(f.readOnly);
      ImGui::TableNextColumn();
      if (!f.editor)
        ImGui::TextColored(kGrey, "-");
      else if (f.drawable)
        ImGui::TextColored(kGreen, "ok");
      else
        ImGui::TextColored(kRed, "UNSUPPORTED");
    }
    ImGui::EndTable();
  }

  void drawComponents() {
    ImGui::InputText("Filter", filter, sizeof(filter));
    ImGui::SameLine();
    ImGui::Checkbox("Only problems", &onlyProblems);
    ImGui::Separator();

    ImGui::BeginChild("components");
    for (const auto &c : ReflectionRegistrar::Get().meta) {
      if (!matchesFilter(c))
        continue;
      auto s = statusOf(c);
      if (onlyProblems && !s.problem() && !s.possibleDuplicate())
        continue;

      std::string title = std::string(c.displayName) + "###" + std::string(c.name);
      ImGui::PushStyleColor(ImGuiCol_Text, s.noUi() ? kRed : ImVec4(1, 1, 1, 1));
      bool open = ImGui::CollapsingHeader(title.c_str());
      ImGui::PopStyleColor();
      if (!open)
        continue;

      ImGui::PushID(c.name.data());
      ImGui::Text("Name: %s   Display: %s", std::string(c.name).c_str(),
                  std::string(c.displayName).c_str());
      ImGui::Text("C++ type: %s   sizeof: %zu", c.typeName, c.size);

      if (s.reflected)
        badge("Reflected UI", kGreen);
      if (s.manual)
        badge("Manual UI", kGreen);
      if (s.noUi())
        badge("NO UI", kRed);
      if (s.possibleDuplicate())
        badge("Duplicate?", kYellow);
      if (s.unsupportedEditor)
        badge("Unsupported field", kYellow);
      ImGui::NewLine();

      drawFieldTable(c);
      ImGui::PopID();
    }
    ImGui::EndChild();
  }

  void drawLive() {
    auto obj = sceneWindow ? sceneWindow->sceneObject : nullptr;
    if (!obj || !obj->instance) {
      ImGui::TextUnformatted("Select an object in the Scene window.");
      return;
    }
    auto *scripts = Engine::Main::ScriptSystem::instance.get();
    ImGui::Text("Object: %s", obj->instance->_name.c_str());
    ImGui::Separator();

    ImGui::BeginChild("live");
    for (auto &[type, comp] : obj->instance->_components) {
      if (!comp)
        continue;
      std::string name(comp->GetName());
      const auto *info = scripts->GetScript(name);
      ImGui::PushID(name.c_str());
      if (!info) {
        ImGui::TextColored(kRed, "%s: not in ScriptSystem", name.c_str());
        ImGui::PopID();
        continue;
      }
      if (ImGui::CollapsingHeader((name + "###live").c_str())) {
        std::string dump;
        try {
          dump = info->toJson(comp.get()).dump(2);
        } catch (const std::exception &e) {
          dump = std::string("toJson threw: ") + e.what();
        }
        if (ImGui::Button("Copy JSON"))
          ImGui::SetClipboardText(dump.c_str());
        ImGui::TextUnformatted(dump.c_str());
      }
      ImGui::PopID();
    }
    ImGui::EndChild();
  }

  //! create -> toJson -> fromJson on a fresh instance -> toJson, compare
  void runSelfTest() {
    selfTest.clear();
    auto *scripts = Engine::Main::ScriptSystem::instance.get();
    for (const auto *info : scripts->All()) {
      if (!info)
        continue;
      SelfTestResult r;
      r.name = std::string(info->name);
      try {
        auto a = info->create();
        auto ja = info->toJson(a.get());
        auto b = info->create();
        auto copy = ja;
        info->fromJson(b.get(), copy);
        auto jb = info->toJson(b.get());
        r.ok = (ja == jb);
        r.message = r.ok ? "round-trip identical"
                         : "JSON differs after fromJson:\nbefore: " + ja.dump() +
                               "\nafter:  " + jb.dump();
      } catch (const std::exception &e) {
        r.ok = false;
        r.message = std::string("threw: ") + e.what();
      }
      selfTest.push_back(std::move(r));
    }
  }

  void drawSelfTest() {
    ImGui::TextWrapped(
        "Creates a fresh instance of every script (constructors run!), "
        "serializes it, loads that JSON into a second instance and compares.");
    if (ImGui::Button("Run self-test"))
      runSelfTest();
    ImGui::Separator();
    for (const auto &r : selfTest) {
      ImGui::TextColored(r.ok ? kGreen : kRed, "%s %s", r.ok ? "PASS" : "FAIL",
                         r.name.c_str());
      if (!r.ok) {
        ImGui::Indent();
        ImGui::TextWrapped("%s", r.message.c_str());
        ImGui::Unindent();
      }
    }
  }

  void draw() override {
    ZoneScoped;
    ImGui::Begin(GetName().c_str());
    if (ImGui::BeginTabBar("reflection_tabs")) {
      if (ImGui::BeginTabItem("Overview")) {
        drawOverview();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Components")) {
        drawComponents();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Live")) {
        drawLive();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Self-test")) {
        drawSelfTest();
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
    ImGui::End();
  }
};
} // namespace Editor