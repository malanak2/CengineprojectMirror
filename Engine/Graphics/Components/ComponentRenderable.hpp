#pragma once

#include "Engine/Graphics/Material.hpp"
#include "Interfaces/IComponent.hpp"
#include <string>

#include "Interfaces/IJson.hpp"
using json = nlohmann::json;
namespace Engine {
namespace Graphics {
struct RenderableDataJson {
public:
  std::string path;
  std::string material_path;
  std::map<std::string, std::vector<float>> uniforms;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RenderableDataJson, material_path, uniforms);

class ComponentRenderable : public IComponent, public IJson {
public:
  /// Only call if you call FromJson right after
  ComponentRenderable();
  ComponentRenderable(std::string path);
  ComponentRenderable(std::string material_path,
                      std::map<std::string, std::vector<float>> uniforms);
  void Setup() override;
  void Update() override;
  void FixedUpdate() override;

  void Save() override;
  void Load() override;

  void FromJson(json &js) override;

  json ToJson() override;

  ComponentRenderable(json &js);
  ENGINE_COMPONENT_TYPE GetType() override;

private:
  std::string path;
  std::string _material_path;
  std::shared_ptr<Material> _material;
  std::map<std::string, std::vector<float>> _uniforms;
};
} // namespace Graphics
} // namespace Engine
