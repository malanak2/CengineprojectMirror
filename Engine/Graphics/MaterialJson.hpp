#pragma once
#include "Shader.hpp"
#include "nlohmann/json.hpp" // IWYU pragma: keep
#include <string>
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
  bool uses_camera;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MaterialJson, name, shaders, uniforms, uses_camera)

} // namespace Graphics
