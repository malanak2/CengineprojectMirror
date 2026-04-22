#include "Program.hpp"
#include "Graphics.hpp"
#include "glad/glad.h"
#include "spdlog/spdlog.h"
#include <cpptrace/basic.hpp>
using namespace Graphics;
Program::Program(std::vector<std::shared_ptr<Shader>> shaders) {
  auto logger = spdlog::get("console");
  unsigned int program;
  program = glCreateProgram();
  for (int i = 0; i < shaders.size(); i++) {
    std::shared_ptr<Shader> shad = shaders[i];
    shaders[i] = shad;
    if (!shad->isValid) {
      logger->warn("Tried to add an invalid shader to a program, ignoring...");
      continue;
    }
    glAttachShader(program, shad->id);
  }
  glLinkProgram(program);
  int success;
  char infoLog[512];
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    logger->error("ERROR::PROGRAM::LINKING_FAILED {}", infoLog);
    cpptrace::generate_trace().print();
    isValid = false;
    return;
  }
  isValid = true;
  id = program;
  for (int i = 0; i < shaders.size(); i++) {
    std::shared_ptr<Shader> shad = shaders[i];
    if (!shad->_reusable) {
      shad->Delete();
      continue;
    }
  }
};
