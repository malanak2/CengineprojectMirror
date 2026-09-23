#pragma once

#include "Interfaces/IComponent.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace Engine {
namespace Graphics {

struct ComponentSocketJson {
  std::string target_object;
  std::string target_bone;
  std::vector<float> position;
  std::vector<float> rotation;
  std::vector<float> scale;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    ComponentSocketJson, target_object, target_bone, position, rotation, scale)

class ComponentSocket : public IComponent {
  REGISTER_CLASS(ComponentSocket);

public:
  void Update() override;
  json ToJson() override;
  void FromJson(json &js) override;

  std::string target_object;
  std::string target_bone;
  glm::vec3 offset_position = glm::vec3(0.0f);
  glm::vec3 offset_rotation = glm::vec3(0.0f); // Euler in degrees
  glm::vec3 offset_scale = glm::vec3(1.0f);
};

} // namespace Graphics
} // namespace Engine
