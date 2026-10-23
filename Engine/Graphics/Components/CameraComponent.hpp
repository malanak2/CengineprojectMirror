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
    glm::mat4 vp = GetViewMatrix() * GetProjMatrix();
    glm::vec3 col1(vp[0][0], vp[1][0], vp[2][0]); //, vp._41
    glm::vec3 col2(vp[0][1], vp[1][1], vp[2][1]); //, vp._42
    glm::vec3 col3(vp[0][2], vp[1][2], vp[2][2]); //, vp._43
    glm::vec3 col4(vp[0][3], vp[1][3],
                   vp[2][3]); //, vp._44
                              // result.left.normal = col4 + col1;
    result.planes.str.right.normal = col4 - col1;
    result.planes.str.bottom.normal = col4 + col2;
    result.planes.str.top.normal = col4 - col2;
    result.planes.str.near.normal = col3;
    result.planes.str.far.normal = col4 - col3;
    result.planes.str.left.distance = vp[3][3] + vp[3][0];
    result.planes.str.right.distance = vp[3][3] - vp[3][0];
    result.planes.str.bottom.distance = vp[3][3] + vp[3][2];
    result.planes.str.top.distance = vp[3][3] - vp[3][1];
    result.planes.str.near.distance = vp[3][2];
    result.planes.str.far.distance = vp[3][3] - vp[3][2];
    for (int i = 0; i < 6; ++i) {
      float mag = 1.0f / glm::sqrt(glm::dot(result.planes.planes[i].normal,
                                            result.planes.planes[i].normal));
      result.planes.planes[i].normal = result.planes.planes[i].normal * mag;
      result.planes.planes[i].distance *= mag;
    }
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
