#pragma once

#include "Interfaces/IIdentifiable.hpp"
#include <memory>
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
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UniformInfo, id, offset)

enum UniformType { Vector, Sampler };
NLOHMANN_JSON_SERIALIZE_ENUM(UniformType, {
                                              {Vector, "vector"},
                                              {Sampler, "sampler"},
                                          })

//!
//! @brief Interface for making uniforms
//!
class IUniform : public IIdentifiable {
public:
  //!
  //! @type Type of the uniform = used to control how it is initialized, and for
  //! memory reasons
  //!
  UniformType type;
  UniformInfo info;
  IUniform(UniformType t, unsigned int id, unsigned int offset,
           std::shared_ptr<std::string> name);
  virtual ~IUniform() = default;
  virtual std::string GetType();

  virtual void Use(unsigned int offset) = 0;
  virtual json ToJson() const;
  virtual void FromJson(json) const;
  static std::shared_ptr<IUniform> CreateUniformFromJson(const json &j);
};

inline void to_json(json &j, const IUniform &u) { j = u.ToJson(); }

inline void from_json(const json &j, IUniform &u) { u.FromJson(j); }

inline void to_json(json &j, const std::shared_ptr<IUniform> &u) {
  if (!u) {
    j = nullptr;
    return;
  }
  j = u->ToJson();
}

inline void from_json(const json &j, std::shared_ptr<IUniform> &u) {
  if (j.is_null()) {
    u = nullptr;
    return;
  }
  u = IUniform::CreateUniformFromJson(j);
}

} // namespace Graphics
} // namespace Engine
