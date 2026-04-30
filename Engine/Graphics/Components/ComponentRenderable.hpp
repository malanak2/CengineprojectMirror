#pragma once

#include "Engine/Graphics/Material.hpp"
#include "Interfaces/IComponent.hpp"
#include "nlohmann/json.hpp"
#include <string>
#include <unordered_map>
using json = nlohmann::json;
namespace Graphics {
struct RenderableDataJson {
public:
  std::string path;
  std::string material_path;
  bool uses_camera;
  std::unordered_map<std::string, std::vector<float>> uniforms;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RenderableDataJson, material_path,
                                   uses_camera, uniforms);

class ComponentRenderable : public IComponent {
public:
  ComponentRenderable(std::string path);
  ComponentRenderable(
      std::string material_path,
      std::unordered_map<std::string, std::vector<float>> uniforms,
      bool uses_camera);
  void Setup() override;
  void Update() override;
  void FixedUpdate() override;

  void Save() override;
  void Load() override;
  void FromJson(json &js);

  json ToJson() override;

  ComponentRenderable(json &js);
  ENGINE_COMPONENT_TYPE GetType() override;

private:
  std::string path;
  std::string _material_path;
  std::shared_ptr<Material> _material;
  std::unordered_map<std::string, std::vector<float>> _uniforms;
  bool uses_camera;
};
} // namespace Graphics
