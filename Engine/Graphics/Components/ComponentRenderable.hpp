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
  std::vector<std::string> disabled_meshes;
};

inline void to_json(json &j, const RenderableDataJson &p) {
  j = json{{"model_path", p.model_path},
           {"material_path", p.material_path},
           {"uniforms", p.uniforms},
           {"disabled_meshes", p.disabled_meshes}};
}

inline void from_json(const json &j, RenderableDataJson &p) {
  j.at("model_path").get_to(p.model_path);
  j.at("material_path").get_to(p.material_path);
  if (j.contains("uniforms")) {
    j.at("uniforms").get_to(p.uniforms);
  }
  if (j.contains("disabled_meshes")) {
    j.at("disabled_meshes").get_to(p.disabled_meshes);
  }
}

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
  std::vector<std::string> disabled_meshes;

  std::vector<bool> GetEnabledMeshes() const;
  bool IsMeshEnabled(const std::string &meshName) const;
  void SetMeshEnabled(const std::string &meshName, bool enabled);

private:
  void FromData(std::string model_path, std::string material_path,
                std::map<std::string, std::shared_ptr<IUniform>> uniforms,
                std::vector<std::string> disabled_meshes = {});
};
} // namespace Graphics
} // namespace Engine
