#pragma once
#include "nlohmann/json.hpp"
#include <string>
namespace Graphics {
struct ShaderJson {
public:
  std::string type;
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
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MaterialJson, name, shaders, uniforms)

} // namespace Graphics
