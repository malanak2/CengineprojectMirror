//
// Created by malan on 21.04.2026.
//

#pragma once
#include <memory>
#include <nlohmann/detail/macro_scope.hpp>
#include <string>
namespace Graphics {
/// Class in which shader info is stored
/// Properties
///   - type
///     Whether vertex, frag, ...
///   - id
///     OpenGL id
enum ShaderType {
  Invalid,
  Vertex,
  Fragment,
};
NLOHMANN_JSON_SERIALIZE_ENUM(ShaderType, {
                                             {Invalid, nullptr},
                                             {Vertex, "vertex"},
                                             {Fragment, "fragment"},
                                         })

class Shader {
public:
  ShaderType type;
  unsigned int id = 0;
  std::string path = "";
  bool isValid = false;
  static std::shared_ptr<Shader> Create(ShaderType type, std::string path,
                                        std::string entrypoint,
                                        bool reusable = false);
  ~Shader();
  bool _reusable = false;
  void Delete();

  Shader(ShaderType type, std::string path, std::string entrypoint,
         bool reusable = false);
};
} // namespace Graphics
