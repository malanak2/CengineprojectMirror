#pragma once

#include "Interfaces/IComponent.hpp"
#include "Interfaces/IIdentifiable.hpp"
#include "Interfaces/IJson.hpp"
#include "Physics/Plane.hpp"
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
class Frustum {
public:
  union Planes {
    struct str {
      Engine::Physics::Plane top;
      Engine::Physics::Plane bottom;
      Engine::Physics::Plane left;
      Engine::Physics::Plane right;
      Engine::Physics::Plane near;
      Engine::Physics::Plane far;
    };
    str str;
    Engine::Physics::Plane planes[6];
    Planes() {}
  };

  Planes planes;
  Frustum() {}
  bool Intersects(std::shared_ptr<Physics::OBB> obb) {
    for (int i = 0; i < 6; ++i) {
      if (planes.planes[i].Classify(obb) < 0) {
        return false;
      }
    }
    return true;
  }
  std::vector<glm::vec3> GetCorners();
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

  Frustum GetFrustum() {
    Frustum result;
    const glm::mat4 vp = GetProjMatrix() * GetViewMatrix();

    auto row = [&vp](int r) {
      return glm::vec4(vp[0][r], vp[1][r], vp[2][r], vp[3][r]);
    };

    const glm::vec4 r0 = row(0);
    const glm::vec4 r1 = row(1);
    const glm::vec4 r2 = row(2);
    const glm::vec4 r3 = row(3);

    auto setPlane = [](Physics::Plane &plane, glm::vec4 p) {
      const float length = glm::length(glm::vec3(p));
      if (length <= 1e-6f) {
        plane.normal = glm::vec3(0.0f);
        plane.distance = 0.0f;
        return;
      }

      p /= length;
      plane.normal = glm::vec3(p);
      plane.distance = -p.w;
    };

    setPlane(result.planes.str.left, r3 + r0);
    setPlane(result.planes.str.right, r3 - r0);
    setPlane(result.planes.str.bottom, r3 + r1);
    setPlane(result.planes.str.top, r3 - r1);
    setPlane(result.planes.str.near, r3 + r2);
    setPlane(result.planes.str.far, r3 - r2);

    return result;
  }
  glm::mat4 GetFrustumMatrix();
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
