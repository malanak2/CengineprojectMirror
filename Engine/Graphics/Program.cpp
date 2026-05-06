#include "Program.hpp"
#include "Engine/Graphics/MaterialJson.hpp"
#include "Graphics.hpp"
#include "Util/LoggerUtil.hpp"
#include "glad/glad.h"
#include "spdlog/spdlog.h"
#include <cpptrace/basic.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
using namespace Engine::Graphics;
using json = nlohmann::json;
Program::Program(std::vector<UniformJson> uniforms_json,
                 std::vector<std::shared_ptr<Shader>> shaders) {
  auto logger = spdlog::get("console");
  unsigned int program;
  program = glCreateProgram();
  for (int i = 0; i < shaders.size(); i++) {
    std::shared_ptr<Shader> shad = shaders[i];
    shaders[i] = shad;
    if (!shad->isValid) {
      SPDLOG_LOGGER_WARN(
          logger, "Tried to add an invalid shader to a program, ignoring...");
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
    SPDLOG_LOGGER_ERROR(logger, "ERROR::PROGRAM::LINKING_FAILED {}", infoLog);
    cpptrace::generate_trace().print();
    isValid = false;
    return;
  }
  isValid = true;
  id = program;
  for (int i = 0; i < shaders.size(); i++) {
    std::shared_ptr<Shader> shad = shaders[i];
    glDetachShader(program, shad->id);
    if (!shad->_reusable) {
      shad->Delete();
      continue;
    }
  }
  for (auto uniform : uniforms_json) {
    unsigned int ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, (int)uniform.size, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, (int)uniform.bind_point, ubo);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    uniforms[uniform.name] = ubo;
    CHECK_GL_ERROR();
    SPDLOG_LOGGER_INFO(spdlog::get("console"),
                       "Created buffer {} with pos {} and size {}",
                       uniform.name, uniform.bind_point, uniform.size);
  }
  CHECK_GL_ERROR();

  /*
  // Link the block to binding point 0
  glUniformBlockBinding(id, blockIndex, 0);
  // Bind the buffer to binding point 0
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
  GLint numUniforms;
  glGetActiveUniformBlockiv(id, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS,
                            &numUniforms);

  std::vector<GLint> uniformIndices(numUniforms);
  glGetActiveUniformBlockiv(id, blockIndex,
                            GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES,
                            uniformIndices.data());

  std::vector<GLint> offsets(numUniforms);
  glGetActiveUniformsiv(id, numUniforms, (GLuint *)uniformIndices.data(),
                        GL_UNIFORM_OFFSET, offsets.data());

  for (int i = 0; i < numUniforms; i++) {
    char name[256];
    GLsizei length;
    glGetActiveUniformName(id, uniformIndices[i], 256, &length, name);

    std::string uniformName(name);

    // Clean up the name (remove "MaterialData." prefix if present)
    size_t dot = uniformName.find('.');
    if (dot != std::string::npos)
      uniformName = uniformName.substr(dot + 1);

    uniforms[uniformName] = {offsets[i]};
    spdlog::get("console")->info("Found Uniform: {} at offset {}", uniformName,
                                 offsets[i]);
  }
  */
};

Program::~Program() {
  glDeleteProgram(id);
  id = 0;
  for (auto ubo : uniforms) {
    glDeleteBuffers(1, &(ubo.second));
  }
  uniforms.clear();
}

/// Assumes program is used
void Program::SetUniform(std::string uniform, float value) {

  if (!uniforms.contains(uniform)) {
    SPDLOG_LOGGER_INFO(spdlog::get("console"), "Uniform not found: {}",
                       uniform);
    return;
  }
  glBindBuffer(GL_UNIFORM_BUFFER, uniforms[uniform]);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float), &value);
  CHECK_GL_ERROR();
}

/// Assumes program is used
void Program::SetUniform(std::string uniform, float v1, float v2, float v3,
                         float v4) {
  if (!uniforms.contains(uniform)) {
    SPDLOG_LOGGER_INFO(spdlog::get("console"), "Uniform not found: {}",
                       uniform);
    return;
  }
  glBindBuffer(GL_UNIFORM_BUFFER, uniforms[uniform]);
  float data[4] = {v1, v2, v3, v4};
  /* spdlog::get("console")->info(
       "Binding ubo {} id {}, setting the data to {} {} {} {}", uniform,
       uniforms[uniform], v1, v2, v3, v4);*/
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(data), data);
  CHECK_GL_ERROR();
}

void Program::SetUniform(std::string key, std::vector<float> values) {
  switch (values.size()) {
  case 1: {
    SetUniform(key, values[0]);
    break;
  }
  case 4: {
    SetUniform(key, values[0], values[1], values[2], values[3]);
    break;
  }
  default: {
    SPDLOG_LOGGER_INFO(spdlog::get("console"),
                       "Unimplemented length of uniform ({})", values.size());
  }
  }
  CHECK_GL_ERROR();
}

void Program::SetUniform(std::string uniform, glm::mat4 mat) {
  CHECK_GL_ERROR();
  if (!uniforms.contains(uniform)) {
    SPDLOG_LOGGER_INFO(spdlog::get("console"), "Uniform not found: {}",
                       uniform);
    return;
  }
  glBindBuffer(GL_UNIFORM_BUFFER, uniforms[uniform]);
  CHECK_GL_ERROR();
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(mat));
  CHECK_GL_ERROR();
}
