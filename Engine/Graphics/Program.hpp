#pragma once
#include "Shader.hpp"
#include <memory>
#include <vector>
namespace Graphics {

/// Class in which program info is stored
/// Properties
///   - id
///     OpenGL id
class Program {
public:
  unsigned int id = 0;
  bool isValid = false;
  /// Takes in a variadic argument of Shader pointers.
  Program(std::vector<std::shared_ptr<Shader>> shaders);
};
} // namespace Graphics
