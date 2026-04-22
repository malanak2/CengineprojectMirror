#include "Shader.hpp"
#include "Graphics.hpp"
#include "Util/FileUtil.hpp"
#include "spdlog/spdlog.h"
#include <cpptrace/basic.hpp>
#include <memory>
using namespace Graphics;
Shader::Shader(ShaderType type, std::string path, std::string entrypoint,
               bool reusable) {
  auto logger = spdlog::get("console");
  if (!GLAD_GL_VERSION_4_6) {
    logger->error("SPIR-V shaders require OpenGL 4.6, but it is not available.");
    isValid = false;
    return;
  }
  std::vector<unsigned char> spirv;
  if (FileUtil::LoadSpirv(path, &spirv) != 0) {
    logger->warn("Failed to load spirv at {}", path);
    isValid = false;
    return;
  }
  switch (type) {
  case Vertex: {
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderBinary(1, &vertexShader, GL_SHADER_BINARY_FORMAT_SPIR_V,
                   spirv.data(), spirv.size());
    glSpecializeShader(vertexShader, (const GLchar *)entrypoint.c_str(), 0,
                       nullptr, nullptr);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
      glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
      logger->error("ERROR::SHADER::VERTEX::COMPILATION_FAILED {}", infoLog);
      logger->error("Shader: {}", path);
      glDeleteShader(vertexShader);
      cpptrace::generate_trace().print();
      isValid = false;
      CHECK_GL_ERROR();
      break;
    }
    id = vertexShader;
    isValid = true;
    _reusable = reusable;
    this->type = type;
    this->path = path;
    break;
  }
  case Fragment: {
    unsigned int fragShader;
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderBinary(1, &fragShader, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(),
                   spirv.size());
    glSpecializeShader(fragShader, (const GLchar *)entrypoint.c_str(), 0,
                       nullptr, nullptr);

    int success;
    char infoLog[512];
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
      glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
      logger->error("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED {}", infoLog);
      glDeleteShader(fragShader);
      cpptrace::generate_trace().print();
      isValid = false;
      CHECK_GL_ERROR();
      return;
    }
    id = fragShader;
    isValid = true;
    _reusable = reusable;
    this->type = type;
    this->path = path;
    break;
  }
  default: {
    logger->error("Tried to create a shader of unimplemented type. ({})",
                  (int)type);
    isValid = false;
    break;
  }
  }
}

std::shared_ptr<Shader> Shader::Create(ShaderType type, std::string path,
                                       std::string entrypoint, bool reusable) {
  auto &cache = type == Shader::ShaderType::Vertex ? Main::vertexShaders
                                                   : Main::fragmentShaders;
  if (cache.contains(path)) {
    return cache[path];
  }
  auto shader = std::make_shared<Shader>(type, path, entrypoint, reusable);
  if (shader->isValid) {
    cache[path] = shader;
  }
  return shader;
}
void Shader::Delete() {
  glDeleteShader(id);
  id = 0;
  if (type == ShaderType::Vertex) {
    Main::vertexShaders.erase(path);
  } else if (type == ShaderType::Fragment) {
    Main::fragmentShaders.erase(path);
  } else {
    spdlog::get("console")->warn("Shader could not be deleted from cache as it "
                                 "is of a type not defined: {}",
                                 (int)type);
  }
}

Shader::~Shader() { Delete(); }

// void Shader::SetUniform(std::string uniform, float value) {};
