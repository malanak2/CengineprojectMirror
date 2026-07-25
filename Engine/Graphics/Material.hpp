#pragma once

#include "Program.hpp"
#include <memory>
namespace Engine {
namespace Graphics {
struct ShaderJson {
public:
  ShaderType type;
  std::string path;
  std::string entrypoint;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ShaderJson, type, path, entrypoint)
struct UniformJson {
public:
  std::string name;
  int bind_point;
  int size;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UniformJson, name, bind_point, size)
struct MaterialJson {
public:
  std::string name;
  std::vector<ShaderJson> shaders;
  std::vector<UniformJson> uniforms;
  std::string texture_path;
  bool uses_camera;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MaterialJson, name, shaders, uniforms,
                                   uses_camera)

class Material {
public:
  std::shared_ptr<Program> program;
  unsigned int vao = 0;
  std::string path = "";
  std::string name = "";
  bool usable = false;
  bool uses_camera;
  std::string texture_path;
  void SetupMaterial();
  void RenderObjects();
  std::vector<std::shared_ptr<ComponentRenderable>> renderableObjects;
  static std::shared_ptr<Material> Create(std::string path);
  Material(std::string path);
  Material();
  ~Material();

private:
  static bool ran_from_create;
};
} // namespace Graphics
} // namespace Engine
