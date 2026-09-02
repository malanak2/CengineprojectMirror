#pragma once

#include "Interfaces/IComponent.hpp"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <string>

#include "Graphics/Program.hpp"

using json = nlohmann::json;
namespace Engine {
namespace Graphics {
class Material;
struct RenderableDataJson {
public:
  std::string path;
  std::string material_path;
  std::map<std::string, std::shared_ptr<IUniform>> uniforms;
  // TODO: Move to model file
  std::vector<float> vertices;
  std::vector<int> indices;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RenderableDataJson, material_path, uniforms,
                                   vertices, indices);

class ComponentRenderable : public IComponent {
public:
  void RenderImGui() override;

  std::string GetName() override;

  /// Only call if you call FromJson right after
  ComponentRenderable(std::shared_ptr<Object> object);
  ComponentRenderable(std::string path, std::shared_ptr<Object> object);
  /*  static std::shared_ptr<ComponentRenderable>
    Create(std::string material_path,
           std::map<std::string, std::vector<float>> uniforms);*/

  static std::shared_ptr<ComponentRenderable>
  Create(json &js, std::shared_ptr<Object> object);
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
  std::shared_ptr<Material> material;
  // TODO: Remove
  std::vector<float> vertices;
  std::vector<int> indices;
  std::string _material_path;
  // Value storing the values of uniforms
  std::map<std::string, std::shared_ptr<IUniform>> _uniforms;

private:
  std::string path;
  /// Material, but...
  void FromData(std::string material_path,
                std::map<std::string, std::shared_ptr<IUniform>> uniforms);
};
} // namespace Graphics
} // namespace Engine
