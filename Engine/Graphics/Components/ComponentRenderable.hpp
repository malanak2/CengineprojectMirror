#pragma once

#include "Graphics/Model.hpp"
#include "Interfaces/IComponent.hpp"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <string>

#include "ScriptSystem.hpp"
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
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RenderableDataJson, model_path, material_path, uniforms, disabled_meshes);

class [[=Attr::Script{}]]ComponentRenderable
    : public IComponent,
      public std::enable_shared_from_this<ComponentRenderable> {
  REGISTER_CLASS(ComponentRenderable);

public:
  ComponentRenderable();
  ComponentRenderable(std::string model_path, std::string material_path,
                      std::shared_ptr<Object> object);

  static std::shared_ptr<ComponentRenderable>
  Create(json &js, std::shared_ptr<Object> object);
  void SetObject(std::shared_ptr<Object> object);
  void FromJson(json &js) override;

  json ToJson() override;
  void EarlySetup() override;
  void LateEnd() override;

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
  bool IsValid() override;

private:
  void FromData(std::string model_path, std::string material_path,
                std::map<std::string, std::shared_ptr<IUniform>> uniforms,
                std::vector<std::string> disabled_meshes = {});
};
} // namespace Graphics
} // namespace Engine
