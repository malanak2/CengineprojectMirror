#pragma once
#include "MaterialJson.hpp"
#include "Shader.hpp"
#include <memory>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <vector>
namespace Graphics {

using json = nlohmann::json;
/// Class in which program info is stored
/// Properties
///   - id
///     OpenGL id
class Program {
public:
  /*struct UniformInfo {
    int offset;
    int size;
  };*/
  unsigned int id = 0;
  bool isValid = false;
  /// Takes in a variadic argument of Shader pointers.
  Program(std::vector<UniformJson> uniforms,
          std::vector<std::shared_ptr<Shader>> shaders);
  ~Program();
  void SetUniform(std::string uniform, float value);
  void SetUniform(std::string uniform, float v1, float v2, float v3, float v4);

  /// Map of name specified in material json, and the index also specified in
  /// there
  std::unordered_map<std::string, unsigned int> uniforms = {};
  //  std::vector<uint8_t> cpuBuffer = {};
  //  void SetUniform(std::string uniform, float value);
  //  void SetUniform(std::string uniform, float value);
};
} // namespace Graphics
