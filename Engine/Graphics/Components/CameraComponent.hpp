#pragma once

#include "Interfaces/IComponent.hpp"
#include "Interfaces/IJson.hpp"
#include "nlohmann/json.hpp" // IWYU pragma: keep
#include <cpptrace/basic.hpp>
#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_float4x4.hpp>

using json = nlohmann::json;

namespace Engine {
namespace Graphics {
struct CameraComponentJson {
public:
};

// NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CameraComponentJson);

class CameraComponent : public IComponent {
public:
  ~CameraComponent() override;

  ENGINE_COMPONENT_TYPE GetType() override;

  CameraComponent(std::shared_ptr<Object> object);

  CameraComponent(std::string path);
  CameraComponent(json &js, std::shared_ptr<Object> object);

  // IComponent
  void Setup() override;

  void Update() override;

  void FixedUpdate() override;

  void Save() override;

  void Load() override;

  // IJson
  json ToJson() override;

  void FromJson(json &js) override;
  glm::mat4 GetProjMatrix();
  glm::mat4 GetViewMatrix();

private:
  glm::mat4 projmat;
  glm::mat4 viewmat;
};
} // namespace Graphics
} // namespace Engine
