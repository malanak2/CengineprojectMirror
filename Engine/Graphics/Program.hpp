#pragma once
#include "Shader.hpp"
#include <memory>
#include <unordered_map>
#include <vector>
namespace Graphics {

/// Class in which program info is stored
/// Properties
///   - id
///     OpenGL id
class Program {
public:
  struct UniformInfo {
    int offset;
    int size;
  };
  unsigned int id = 0;
  bool isValid = false;
  Program(std::vector<std::shared_ptr<Shader>> shaders);
  ~Program();
  void SetUniform(std::string uniform, float value);
  void SetUniform(std::string uniform, float v1, float v2, float v3, float v4);

  std::unordered_map<std::string, UniformInfo> uniforms = {};
  std::vector<uint8_t> cpuBuffer = {};
  unsigned int ubo;
  //  void SetUniform(std::string uniform, float value);
  //  void SetUniform(std::string uniform, float value);
};
} // namespace Graphics
