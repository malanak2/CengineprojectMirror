#pragma once
#include "MaterialJson.hpp"
#include "Shader.hpp"
#include "glm/glm.hpp"
#include <memory>
#include <nlohmann/json.hpp>
#include <vector>
namespace Engine::Graphics {

using json = nlohmann::json;
/// Class in which program info is stored
/// Properties
///   - id
///     OpenGL id
struct UniformInfo {
public:
  unsigned int id;
  unsigned int offset;
};
  enum UniformType {
    Vector,
    Sampler
};
struct Uniform {
public:
  UniformType type;
  UniformInfo info;
  std::shared_ptr<void> Data;
};

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
          std::vector<std::shared_ptr<Shader>> shaders, bool uses_camera = false);
  ~Program();
  void SetUniform(std::string uniform, float value, bool camera);
  void SetUniform(std::string uniform, float v1, float v2, float v3, float v4,
                  bool camera);
  void SetUniform(std::string uniform, std::vector<float> values, bool camera);
  void SetUniform(std::string uniform, glm::mat4 mat, bool camera);

  /// Map of name specified in material json, and the index also specified in
  /// there
  std::map<std::string, Uniform> uniforms = {};
  std::map<int, std::string> uniforms_info = {};
  bool _uses_camera = false;
  //  std::vector<uint8_t> cpuBuffer = {};
  //  void SetUniform(std::string uniform, float value);
  //  void SetUniform(std::string uniform, float value);
private:
  unsigned int GetUniformOffset(std::string uniform, bool camera);
};
} // namespace Engine::Graphics
