#pragma once

#include "Interfaces/IComponent.hpp"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <string>

using json = nlohmann::json;
namespace Engine {
namespace Graphics {
struct RenderableDataJson {
public:
  std::string path;
  std::string material_path;
  std::map<std::string, std::vector<float>> uniforms;
  // TODO: Move to model file
  std::vector<float> vertices;
  std::vector<float> indices;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RenderableDataJson, material_path, uniforms,
                                   vertices, indices);

class ComponentRenderable : public IComponent {
public:
  /// Only call if you call FromJson right after
  ComponentRenderable();
  ComponentRenderable(std::string path);
  /*  static std::shared_ptr<ComponentRenderable>
    Create(std::string material_path,
           std::map<std::string, std::vector<float>> uniforms);*/

  static std::shared_ptr<ComponentRenderable> Create(json &js);
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
  std::map<std::string, std::vector<float>> _uniforms;
  unsigned int vao = 0;
  unsigned int vbo = 0;
  unsigned int ebo = 0;
  /// Material, but...
  std::shared_ptr<void>
  FromData(std::string material_path,
           std::map<std::string, std::vector<float>> uniforms);
  // TODO: Remove
  std::vector<float> vertices;
  std::vector<float> indices;
};
} // namespace Graphics
} // namespace Engine
