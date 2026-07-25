#pragma once
#include "Graphics/Texture.hpp"
#include "Shader.hpp"
#include "glm/glm.hpp" // IWYU pragma: keep
#include <memory>
#include <nlohmann/json.hpp>
#include <vector>
namespace Engine::Graphics {
struct UniformJson;
using json = nlohmann::json;
/// Class in which program info is stored
/// Properties
///   - opengl id
///   - byte offset in uniform
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
  unsigned int id = 0;
  bool isValid = false;
  Program(std::vector<UniformJson> uniforms,
          std::vector<std::shared_ptr<Shader>> shaders, std::string texpath,
          bool uses_camera = false);
  ~Program();
  void SetUniform(std::string uniform, float value, bool camera);
  void SetUniform(std::string uniform, float v1, float v2, float v3, float v4,
                  bool camera);
  void SetUniform(std::string uniform, std::vector<float> values, bool camera);
  void SetUniform(std::string uniform, glm::mat4 mat, bool camera);

  void BindTexture2D(Texture tex);

  /// Map of name specified in material json, and the index also specified in
  /// there
  std::map<std::string, Uniform> uniforms = {};
  std::map<int, std::string> uniforms_info = {};
  bool _uses_camera = false;

private:
  unsigned int GetUniformOffset(std::string uniform, bool camera);
};
} // namespace Engine::Graphics
