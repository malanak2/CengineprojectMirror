#include "Shader.hpp"
#include "Graphics.hpp"
#include "spdlog/spdlog.h"
#include <cpptrace/basic.hpp>
#include <memory>
using namespace Graphics;
Shader::Shader(ShaderType type, std::string source, bool reusable) {
  auto logger = spdlog::get("console");
  switch (type) {
  case Vertex: {
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char *out_c = source.c_str();
    glShaderSource(vertexShader, 1, &out_c, NULL);
    glCompileShader(vertexShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
      logger->error("ERROR::SHADER::VERTEX::COMPILATION_FAILED {}", infoLog);
      logger->error("Shader: {}", source);
      cpptrace::generate_trace().print();
      isValid = false;
      break;
    }
    id = vertexShader;
    isValid = true;
    _reusable = reusable;
    this->type = type;
    this->source = source;
    break;
  }
  case Fragment: {
    unsigned int fragShader;
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char *frag_out_c = source.c_str();
    glShaderSource(fragShader, 1, &frag_out_c, NULL);
    glCompileShader(fragShader);
    int success;
    char infoLog[512];
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
      logger->error("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED {}", infoLog);
      cpptrace::generate_trace().print();
      isValid = false;
      return;
    }
    id = fragShader;
    isValid = true;
    _reusable = reusable;
    this->type = type;
    this->source = source;
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

std::shared_ptr<Shader> Shader::Create(ShaderType type, std::string source,
                                       bool reusable) {
  auto &cache = type == Shader::ShaderType::Vertex ? Main::vertexShaders
                                                   : Main::fragmentShaders;
  if (cache.contains(source)) {
    return cache[source];
  }
  auto shader = std::make_shared<Shader>(type, source, reusable);
  if (shader->isValid) {
    cache[source] = shader;
  }
  return shader;
}
void Shader::Delete() {
  glDeleteShader(id);
  if (type == ShaderType::Vertex) {
    Main::vertexShaders.erase(source);
  } else if (type == ShaderType::Fragment) {
    Main::fragmentShaders.erase(source);
  } else {
    spdlog::get("console")->warn("Shader could not be deleted from cache as it "
                                 "is of a type not defined: {}",
                                 (int)type);
  }
}

Shader::~Shader() { Delete(); }

// void Shader::SetUniform(std::string uniform, float value) {};
