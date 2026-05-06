#pragma once

#include "Interfaces/IComponent.hpp"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <memory>
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
  std::vector<int> indices;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RenderableDataJson, material_path, uniforms,
                                   vertices, indices);

class ComponentRenderable : public IComponent {
public:
  /// Only call if you call FromJson right after
  ComponentRenderable(std::shared_ptr<void> object);
  ComponentRenderable(std::string path, std::shared_ptr<void> object);
  /*  static std::shared_ptr<ComponentRenderable>
    Create(std::string material_path,
           std::map<std::string, std::vector<float>> uniforms);*/

  static std::shared_ptr<ComponentRenderable>
  Create(json &js, std::shared_ptr<void> object);
  void Setup() override;
  void Update() override;
  void FixedUpdate() override;

  void Save() override;
  void Load() override;

  void FromJson(json &js) override;

  json ToJson() override;

  ComponentRenderable(json &js);
  ENGINE_COMPONENT_TYPE GetType() override;
  unsigned int vao = 0;
  unsigned int vbo = 0;
  unsigned int ebo = 0;
  // TODO: Remove
  std::vector<float> vertices;
  std::vector<int> indices;
  std::map<std::string, std::vector<float>> _uniforms;
  std::string _material_path;
  std::shared_ptr<void> object;

private:
  std::string path;

  /// Material, but...
  std::shared_ptr<void>
  FromData(std::string material_path,
           std::map<std::string, std::vector<float>> uniforms);
};
} // namespace Graphics
} // namespace Engine
