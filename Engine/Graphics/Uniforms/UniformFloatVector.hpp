#pragma once

#include "Graphics/Interfaces/IUniform.hpp"
#include <nlohmann/detail/macro_scope.hpp>
namespace Engine {
namespace Graphics {
class UniformFloatVector : public IUniform {
public:
  void Use(unsigned int offset) override;
  void RenderImGui() override;
  UniformFloatVector(UniformType t, unsigned int id, unsigned int offset,
                     std::shared_ptr<std::string> name,
                     std::vector<float> data);
  std::vector<float> data;
};

NLOHMANN_DEFINE_DERIVED_TYPE_NON_INTRUSIVE(UniformFloatVector, IUniform, data)
} // namespace Graphics
} // namespace Engine
