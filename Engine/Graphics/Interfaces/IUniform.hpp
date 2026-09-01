#pragma once

#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace Engine {
namespace Graphics {
/// Class in which program info is stored
/// Properties
///   - opengl id
///   - byte offset in uniform
struct UniformInfo {
public:
  unsigned int id;
  unsigned int offset;
  std::shared_ptr<std::string> name;
};

enum UniformType { Vector, Sampler };

class IUniform {
public:
  UniformType type;
  UniformInfo info;
  IUniform(UniformType t, unsigned int id, unsigned int offset,
           std::shared_ptr<std::string> name);
  virtual void RenderImGui();

  virtual void Use(unsigned int offset) = 0;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(IUniform, type, info)
} // namespace Graphics
} // namespace Engine
