#include "Program.hpp"
#include "Graphics.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/Uniforms/UniformFloatVector.hpp"
#include "spdlog/spdlog.h"
#include "tracy/TracyOpenGL.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <stacktrace>
#include <tracy/Tracy.hpp>
#include <vector>
using namespace Engine::Graphics;
using json = nlohmann::json;
Program::Program(std::vector<UniformJson> uniforms_json,
                 std::vector<std::shared_ptr<Shader>> shaders,
                 std::string texpath, bool uses_camera) {

  ZoneScoped;
  _uses_camera = uses_camera;
  auto logger = spdlog::get("console");
  unsigned int program = glCreateProgram();
  std::vector<std::shared_ptr<Shader>> attached; // only these may be detached
  for (auto &shad : shaders) {
    if (!shad || !shad->isValid) {
      SPDLOG_LOGGER_WARN(
          logger, "Tried to add an invalid shader to a program, ignoring...");
      continue;
    }
    glAttachShader(program, shad->id);
    attached.push_back(shad);
  }
  glLinkProgram(program);

  auto releaseShaders = [&] {
    for (auto &shad : attached)
      glDetachShader(program, shad->id);
    for (auto &shad : attached)
      if (!shad->_reusable)
        shad->Delete();
  };

  int success = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    int logLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    std::string infoLog(static_cast<size_t>(std::max(logLength, 1)), '\0');
    glGetProgramInfoLog(program, static_cast<GLsizei>(infoLog.size()), nullptr,
                        infoLog.data());
    SPDLOG_LOGGER_ERROR(logger, "ERROR::PROGRAM::LINKING_FAILED {}", infoLog);
    releaseShaders();
    glDeleteProgram(program); // don't leak the failed program
    isValid = false;
    return;
  }
  isValid = true;
  id = program;
  releaseShaders();
  glGenBuffers(1, &this->ubo);
  const unsigned int ubo = this->ubo;
  glBindBuffer(GL_UNIFORM_BUFFER, ubo);
  unsigned int total_size = _uses_camera ? 128 : 0;
  for (auto uniform : uniforms_json) {
    UniformInfo ui;
    ui.id = ubo;
    ui.offset = (unsigned int)uniform.size;
    ui.name = std::make_shared<std::string>(uniform.name);
    uniforms[uniform.name] = std::make_shared<UniformFloatVector>(
        Vector, ubo, ui.offset, std::make_shared<std::string>(uniform.name),
        std::vector<float>());
    uniforms_info[uniform.bind_point] = uniform.name;
    total_size += ui.offset;
    CHECK_GL_ERROR();
    SPDLOG_LOGGER_INFO(spdlog::get("console"),
                       "Created buffer {} with pos {} and size {}",
                       uniform.name, uniform.bind_point, uniform.size);
  }
  if (texpath != "") {
    glActiveTexture(GL_TEXTURE1);
    tex = Texture::Create(texpath);
    glBindTexture(GL_TEXTURE_2D, tex->texture);
  }

  glBufferData(GL_UNIFORM_BUFFER, total_size, NULL, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  CHECK_GL_ERROR();
}

Program::~Program() {
  ZoneScoped;
  if (id != 0) {
    glDeleteProgram(id);
    id = 0;
  }
  if (ubo != 0) {
    glDeleteBuffers(1, &ubo);
    ubo = 0;
  }
  uniforms.clear();
}

/// Assumes program is used
void Program::SetUniform(const std::string &uniform, float value, bool camera) {

  ZoneScoped;
  if (!uniforms.contains(uniform)) {
    SPDLOG_LOGGER_INFO(spdlog::get("console"), "Uniform not found: {}",
                       uniform);
    return;
  }
  glBindBuffer(GL_UNIFORM_BUFFER, uniforms[uniform]->info.id);
  glBufferSubData(GL_UNIFORM_BUFFER, GetUniformOffset(uniform, camera),
                  sizeof(float), &value);
  CHECK_GL_ERROR();
}
void Engine::Graphics::Program::SetUniform(const std::string &uniform,
                                           int value, bool camera) {
  ZoneScoped;
  if (!uniforms.contains(uniform)) {
    SPDLOG_LOGGER_INFO(spdlog::get("console"), "Uniform not found: {}",
                       uniform);
    return;
  }
  glBindBuffer(GL_UNIFORM_BUFFER, uniforms[uniform]->info.id);
  glBufferSubData(GL_UNIFORM_BUFFER, GetUniformOffset(uniform, camera),
                  sizeof(float), &value);
}

/// Assumes program is used
void Program::SetUniform(const std::string &uniform, float v1, float v2,
                         float v3, float v4, bool camera) {
  ZoneScoped;
  if (!uniforms.contains(uniform)) {
    SPDLOG_LOGGER_INFO(spdlog::get("console"), "Uniform not found: {}",
                       uniform);
    return;
  }
  glBindBuffer(GL_UNIFORM_BUFFER, uniforms[uniform]->info.id);
  float data[4] = {v1, v2, v3, v4};
  /* spdlog::get("console")->info(
       "Binding ubo {} id {}, setting the data to {} {} {} {}", uniform,
       uniforms[uniform], v1, v2, v3, v4);*/
  glBufferSubData(GL_UNIFORM_BUFFER, GetUniformOffset(uniform, camera),
                  sizeof(data), data);
  CHECK_GL_ERROR();
}

void Program::SetUniform(const std::string &key, std::vector<float> values,
                         bool camera) {
  ZoneScoped;
  switch (values.size()) {
  case 1: {
    SetUniform(key, values[0], camera);
    break;
  }
  case 4: {
    SetUniform(key, values[0], values[1], values[2], values[3], camera);
    break;
  }
  default: {
    SPDLOG_LOGGER_INFO(spdlog::get("console"),
                       "Unimplemented length of uniform ({})", values.size());
  }
  }
  CHECK_GL_ERROR();
}

void Program::SetUniform(const std::string &uniform, glm::mat4 mat,
                         bool camera) {
  ZoneScoped;
  CHECK_GL_ERROR();
  if (!uniforms.contains(uniform)) {
    SPDLOG_LOGGER_INFO(spdlog::get("console"), "Uniform not found: {}",
                       uniform);
    return;
  }
  glBindBuffer(GL_UNIFORM_BUFFER, uniforms[uniform]->info.id);
  CHECK_GL_ERROR();

  glBufferSubData(GL_UNIFORM_BUFFER, GetUniformOffset(uniform, camera),
                  sizeof(glm::mat4), glm::value_ptr(mat));
  CHECK_GL_ERROR();
}
void Engine::Graphics::Program::BindTexture2D(const Texture &tex) {
  ZoneScoped;
  glBindTexture(GL_TEXTURE_2D, tex.texture);
}

unsigned int Program::GetUniformOffset(const std::string &uniform,
                                       bool camera) {
  ZoneScoped;
  unsigned int offset = 0;
  if (camera)
    offset = 128;
  for (auto [key, val] : uniforms_info) {
    if (val == uniform)
      return offset;
    offset += uniforms[val]->info.offset;
  }
  return -1;
}
void Engine::Graphics::Program::Setup() {
  ZoneScopedN("Program::Setup");
  TracyGpuZone("Program::Setup");
  glUseProgram(id);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
  if (tex) {
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex->texture);
  }
}
