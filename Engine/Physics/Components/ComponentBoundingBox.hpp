#pragma once

#include "Interfaces/IComponent.hpp"
#include "Interfaces/IIdentifiable.hpp"
#include "Object.hpp"
#include "Util/LoggerUtil.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <imgui.h>
#include <nlohmann/detail/macro_scope.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <memory>
namespace Engine {
namespace Physics {
class CollisionManifold;
class Line;
class Plane;
// SZAUER, Gabor. Game Physics Cookbook. In: . S. 152-192. ISBN 978-1787120815.
class OBB : public std::enable_shared_from_this<OBB> {
public:
  glm::vec3 position;
  glm::vec3 size;
  glm::mat3 orientation;

  std::pair<double, double> GetInterval(glm::vec3 axis);
  bool OverlapOnAxis(std::shared_ptr<OBB> obb1, std::shared_ptr<OBB> obb2,
                     const glm::vec3 &axis);
  bool Collides(std::shared_ptr<OBB> other);
  glm::mat4 GetModelMatrix() const {
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 rotation = glm::mat4(orientation);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), size);

    return translation * rotation * scale;
  }
  std::vector<glm::vec3> GetVertices();
  std::vector<Line> GetEdges();
  std::vector<Plane> GetPlanes();
  std::pair<bool, glm::vec3> ClipToPlane(Plane plane, Line line);
  std::vector<glm::vec3> ClipEdgesToOBB(std::vector<Line> edges);
  std::pair<bool, float> PenetrationDepth(std::shared_ptr<OBB> o2,
                                          glm::vec3 axis);
  std::shared_ptr<CollisionManifold>
  FindCollisionFeatures(std::shared_ptr<OBB> B);
  bool PointInOBB(glm::vec3 point);
};
class ComponentBBoxJson {
public:
  std::vector<float> position_offset; // Offset from center of object
  std::vector<float> size;            // Unique
  std::vector<float> orientation;     // Offset from object
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ComponentBBoxJson, position_offset, size,
                                   orientation);
class ComponentBoundingBox
    : public IComponent,
      public std::enable_shared_from_this<ComponentBoundingBox> {
  REGISTER_CLASS(ComponentBoundingBox);

public:
  json ToJson() override {
    ComponentBBoxJson js;
    js.size = std::vector<float>{this->size[0], this->size[1], this->size[2]};
    js.position_offset =
        std::vector<float>{this->position_offset[0], this->position_offset[1],
                           this->position_offset[2]};
    js.orientation =
        std::vector<float>{orientation_offset.w, orientation_offset.x,
                           orientation_offset.y, orientation_offset.z};
    return js;
  }

  void FromJson(json &js) override {
    if (!js.contains("size") || !js.contains("position_offset") ||
        !js.contains("orientation")) {
      SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER,
                         "An Object has saved invalid OBB - reset");
      return;
    }
    ComponentBBoxJson cb = js;

    size = glm::vec3(cb.size[0], cb.size[1], cb.size[2]);
    position_offset = glm::vec3(cb.position_offset[0], cb.position_offset[1],
                                cb.position_offset[2]);
    orientation_offset = glm::quat(cb.orientation[0], cb.orientation[1],
                                   cb.orientation[2], cb.orientation[3]);
  }
  void Setup() override;
  void End() override;
  void Update() override;
  std::shared_ptr<OBB> GetOBB();
  bool Collides(std::shared_ptr<ComponentBoundingBox> other) {
    return GetOBB()->Collides(other->GetOBB());
  }
  void InvalidateBB() { obb = nullptr; }

  glm::vec3 position_offset = {0, 0, 0};
  glm::quat orientation_offset = glm::identity<glm::quat>();
  glm::vec3 size = {
      1,
      1,
      1,
  };
  bool FollowsPausedExecution() override { return false; }

private:
  std::shared_ptr<OBB> obb = nullptr;
};

} // namespace Physics
} // namespace Engine
