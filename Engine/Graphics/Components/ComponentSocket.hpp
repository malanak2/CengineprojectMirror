#pragma once

#include "Interfaces/IComponent.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "ScriptSystem.hpp"

namespace Engine {
namespace Graphics {


class [[=Attr::Script{}]] ComponentSocket : public IComponent {
  REGISTER_CLASS(ComponentSocket);

public:
  void Update() override;

  [[=Attr::Serialize{}]]std::string target_object;
  [[=Attr::Serialize{}]]std::string target_bone;
  [[=Attr::Serialize{}]]glm::vec3 offset_position = glm::vec3(0.0f);
  [[=Attr::Serialize{}]]glm::vec3 offset_rotation = glm::vec3(0.0f); // Euler in degrees
  [[=Attr::Serialize{}]]glm::vec3 offset_scale = glm::vec3(1.0f);
};

} // namespace Graphics
} // namespace Engine
