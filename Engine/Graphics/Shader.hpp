//
// Created by malan on 21.04.2026.
//

#pragma once
#include <memory>
#include <string>
namespace Graphics {
/// Class in which shader info is stored
/// Properties
///   - type
///     Whether vertex, frag, ...
///   - id
///     OpenGL id
class Shader {
public:
  enum ShaderType {
    Invalid,
    Vertex,
    Fragment,
  };
  ShaderType type;
  unsigned int id = 0;
  std::string source = "";
  bool isValid = false;
  static std::shared_ptr<Shader> Create(ShaderType type, std::string source,
                                        bool reusable = false);
  ~Shader();
  bool _reusable = false;
  void Delete();

  Shader(ShaderType type, std::string source, bool reusable = false);
};
} // namespace Graphics
