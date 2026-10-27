#pragma once

#include "Interfaces/IComponent.hpp"
#include "Interfaces/IIdentifiable.hpp"
#include "Object.hpp"
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

  std::pair<double, double> GetInterval(const glm::vec3 &axis) const;
  bool OverlapOnAxis(const std::shared_ptr<OBB> &obb1,
                     const std::shared_ptr<OBB> &obb2,
                     const glm::vec3 &axis) const;
  bool Collides(const std::shared_ptr<OBB> &other);
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
  std::vector<float> position_offset;
  std::vector<float> size;
  std::vector<float> orientation;
  bool isTrigger;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ComponentBBoxJson, position_offset, size,
                                   orientation, isTrigger);

class ComponentBoundingBox
    : public IComponent,
      public std::enable_shared_from_this<ComponentBoundingBox> {
  REGISTER_CLASS(ComponentBoundingBox);

public:
  json ToJson() override;

  void FromJson(json &js) override;
  void EarlySetup() override;
  void LateEnd() override;
  void Update() override;
  std::shared_ptr<OBB> GetOBB();
  bool Collides(const std::shared_ptr<ComponentBoundingBox> &other) {
    return GetOBB()->Collides(other->GetOBB());
  }
  void InvalidateBB() { obb = nullptr; }
  bool isTrigger = false;
  using ContactCallback =
      std::function<void(std::shared_ptr<ComponentBoundingBox> other)>;

  ContactCallback OnCollisionEnter;
  ContactCallback OnCollision;
  ContactCallback OnCollisionLeave;

  ContactCallback OnTriggerEnter;
  ContactCallback OnTrigger;
  ContactCallback OnTriggerLeave;
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
