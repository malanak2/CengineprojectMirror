#pragma once

#include "Interfaces/IComponent.hpp"
#include "Interfaces/IIdentifiable.hpp"
#include "ScriptSystem.hpp"
namespace Engine
{
  class [[=Attr::Script{}]] InvalidComponent : public IComponent {
  public:
    static std::string_view GetNameS() { return "InvalidComponent"; }
    std::string_view GetName() override {
      return originalName.empty() ? GetNameS() : std::string_view(originalName);
    }
    json ToJson() override { return raw; }
    void FromJson(json &js) override { raw = js; }
    bool IsValid() override { return false; }

    std::string originalName;
    json raw;
  };
}; // namespace Engine
