#include "Shader.hpp"
#include "Graphics.hpp"
#include "spdlog/spdlog.h"
using namespace Graphics;
Shader::Shader(ShaderType type, std::string source, bool reusable) {
  auto logger = spdlog::get("console");
  switch (type) {
  case Vertex: {
    if (Main::vertexShaders.contains(source)) {
      auto shader = Main::vertexShaders.at(source);
      this->_reusable = shader->_reusable;
      this->isValid = shader->isValid;
      this->id = shader->id;
      this->type = shader->type;
      return;
    }
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
      isValid = false;
      break;
    }
    id = vertexShader;
    isValid = true;
    _reusable = reusable;
    Main::vertexShaders[source] = this;
    break;
  }
  case Fragment: {
    if (Main::fragmentShaders.contains(source)) {
      auto shader = Main::fragmentShaders.at(source);
      this->_reusable = shader->_reusable;
      this->isValid = shader->isValid;
      this->id = shader->id;
      this->type = shader->type;
      return;
    }
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
      isValid = false;
      return;
    }
    id = fragShader;
    isValid = true;
    _reusable = reusable;
    Main::fragmentShaders[source] = this;
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
