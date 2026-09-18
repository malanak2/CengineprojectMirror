#pragma once

#include "Graphics/Interfaces/IUniform.hpp"
#include "Interfaces/IIdentifiable.hpp"
#include <nlohmann/detail/macro_scope.hpp>
namespace Engine {
namespace Graphics {
//!
//! @brief Class for handling vector float uniforms
//!
class UniformFloatVector : public IUniform {
public:
  REGISTER_CLASS(UniformFloatVector);
  void Use(unsigned int offset) override;
  std::string GetType() override;
  UniformFloatVector(UniformType t, unsigned int id, unsigned int offset,
                     std::shared_ptr<std::string> name,
                     std::vector<float> data);
  std::vector<float> data;
  json ToJson() const override;
};

NLOHMANN_DEFINE_DERIVED_TYPE_NON_INTRUSIVE(UniformFloatVector, IUniform, data)
} // namespace Graphics
} // namespace Engine
