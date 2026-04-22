//
// Created by malan on 21.04.2026.
//

#pragma once
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
    Vertex,
    Fragment,
  };
  ShaderType type;
  unsigned int id = 0;
  std::string source = "";
  bool isValid = false;
  Shader(ShaderType type, std::string source, bool reusable = false);
  bool _reusable = false;
  void Delete();
};
} // namespace Graphics
