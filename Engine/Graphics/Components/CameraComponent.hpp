#pragma once

#include "Interfaces/IComponent.hpp"
#include "Interfaces/IJson.hpp"
#include "nlohmann/json.hpp" // IWYU pragma: keep
#include <cpptrace/basic.hpp>

using json = nlohmann::json;

namespace Engine {
namespace Graphics {
struct CameraComponentJson {
public:
};

// NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CameraComponentJson);

class CameraComponent : public IComponent, public IJson {
public:
  ~CameraComponent() override;

  ENGINE_COMPONENT_TYPE GetType() override;

  CameraComponent();

  CameraComponent(std::string path);

  // IComponent
  void Setup() override;

  void Update() override;

  void FixedUpdate() override;

  void Save() override;

  void Load() override;

  // IJson
  json ToJson() override;

  void FromJson(json &js) override;

private:
};
} // namespace Graphics
} // namespace Engine
