#pragma once

#include "Graphics/Model.hpp"
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
  std::string model_path;
  std::string material_path;
  std::map<std::string, std::shared_ptr<IUniform>> uniforms;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RenderableDataJson, model_path,
                                   material_path, uniforms);

class ComponentRenderable
    : public IComponent,
      public std::enable_shared_from_this<ComponentRenderable> {
  REGISTER_CLASS(ComponentRenderable);

public:
  ComponentRenderable();
  ComponentRenderable(std::string model_path, std::string material_path,
                      std::shared_ptr<Object> object);

  static std::shared_ptr<ComponentRenderable>
  Create(json &js, std::shared_ptr<Object> object);
  void Setup() override;
  void Update() override;
  void FixedUpdate() override;
  void SetObject(std::shared_ptr<Object> object);
  void FromJson(json &js) override;

  json ToJson() override;

  ComponentRenderable(json &js);
  std::shared_ptr<Material> material;
  std::shared_ptr<Model> model;
  std::string _material_path;
  std::string _model_path;
  // Value storing the values of uniforms
  std::map<std::string, std::shared_ptr<IUniform>> _uniforms;

private:
  void FromData(std::string model_path, std::string material_path,
                std::map<std::string, std::shared_ptr<IUniform>> uniforms);
};
} // namespace Graphics
} // namespace Engine
