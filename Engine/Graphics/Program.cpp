#include "Program.hpp"
#include "Graphics.hpp"
#include "Shader.hpp"
#include "glad/glad.h"
#include "spdlog/spdlog.h"
#include <cstdarg>
using namespace Graphics;
Program::Program(int count, ...) {
  auto logger = spdlog::get("console");
  logger->info("Creating new program with {} shaders", count);
  unsigned int program;
  program = glCreateProgram();
  va_list args;
  va_start(args, count);
  Shader *shaders[count];
  for (int i = 0; i < count; i++) {
    Shader *shad = va_arg(args, Shader *);
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
    isValid = false;
    return;
  }
  isValid = true;
  id = program;
  for (int i = 0; i < count; i++) {
    Shader *shad = shaders[i];
    if (!shad->_reusable) {
      shad->Delete();
      continue;
    }
  }
};
