#pragma once

#include "Interfaces/IComponent.hpp"
#include "Interfaces/IIdentifiable.hpp"
#include "Object.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <imgui.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <memory>
namespace Engine {
namespace Physics {
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
};
class ComponentBoundingBox
    : public IComponent,
      public std::enable_shared_from_this<ComponentBoundingBox> {
  REGISTER_CLASS(ComponentBoundingBox);

public:
  json ToJson() override { return {}; }

  void FromJson(json &js) override { return; }
  void Setup() override;
  void End() override;
  void Update() override;
  std::shared_ptr<OBB> GetOBB();
  bool Collides(std::shared_ptr<ComponentBoundingBox> other) {
    return GetOBB()->Collides(other->GetOBB());
  }

private:
  std::shared_ptr<OBB> obb = nullptr;
};

} // namespace Physics
} // namespace Engine
