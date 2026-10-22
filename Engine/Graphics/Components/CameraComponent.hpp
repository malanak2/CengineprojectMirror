#pragma once

#include "Interfaces/IComponent.hpp"
#include "Interfaces/IIdentifiable.hpp"
#include "Interfaces/IJson.hpp"
#include "nlohmann/json.hpp" // IWYU pragma: keep
#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/quaternion.hpp>

using json = nlohmann::json;

namespace Engine {
namespace Graphics {
struct CameraComponentJson {
public:
  float near;
  float far;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CameraComponentJson, near, far);

class CameraComponent : public IComponent {
  REGISTER_CLASS(CameraComponent);

public:
  ~CameraComponent() override;

  CameraComponent() {}
  CameraComponent(std::shared_ptr<Object> object);

  CameraComponent(std::string path);
  CameraComponent(json &js, std::shared_ptr<Object> object);

  // IComponent
  void EarlySetup() override;

  void Update() override;

  // IJson
  json ToJson() override;

  // We do not need our camera to pause during execution paused - it only
  // updates its vectors
  bool FollowsPausedExecution() override { return false; }
  void FromJson(json &js) override;

  glm::mat4 GetProjMatrix();
  glm::mat4 GetViewMatrix();

  float near, far;

  glm::mat4 projmat;
  glm::mat4 viewmat;
  glm::vec3 front;
  glm::vec3 right;
  glm::vec3 up;

private:
  void recalculateMatrixes();

  bool isUpToDate();
  glm::quat oldrot = {};
  glm::vec3 oldpos = {};
  int w, h;
};
} // namespace Graphics
} // namespace Engine
